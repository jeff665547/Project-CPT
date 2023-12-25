#include <iostream>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CPT/format/cen.hpp>
#include <CPT/format/cube.hpp>
#include <CPT/engine/data_pool/chip_layout.hpp>
#include <CPT/algorithm/quantile_normalization_for_cube.hpp>
#include <CPT/algorithm/transformations.hpp>

std::string get_sample_name(const std::string &path)
{
    std::vector<std::string> path_file;
    boost::iter_split(path_file, path, boost::algorithm::first_finder("/"));

    // return path_file[path_file.size() - 3];
    // /home/joyel/WorkDir/DogBanff/2023-08-09-CanfamDataset/20230118/101_20230116120646/grid/array.cen

    std::vector<std::string> sample;
    boost::iter_split(sample, path_file[path_file.size() - 1], boost::algorithm::first_finder("."));

    return sample[0];
}

int main(int argc, const char *argv[])
{
    nucleona::parallel::set_max_thread_num((int)std::thread::hardware_concurrency());

    if (argc != 4)
    {
        std::cout << "./marker_probe_export [cad file] [anno.csv] [cen files list]" << std::endl;
        return 1;
    }

    int num_rows = 496, num_cols = 496;
    std::vector<std::string> order{"Banff_AM1E_red", "Banff_AM3_green"};

    auto cad_file = std::string(argv[1]);
    auto anno_file = std::string(argv[2]);
    auto cen_files = std::string(argv[3]);

    auto line = std::string();
    auto file_in = std::ifstream(anno_file);
    auto splits = std::vector<std::string>();

    cpt::engine::data_pool::GenotypingCadChipLayout cad;
    cad.read_chip_layout(cad_file);

    auto x_map = std::map<int, int>();
    auto y_map = std::map<int, int>();
    auto pid_map = std::map<int, std::vector<int>>();

    while (std::getline(file_in, line))
    {
        if (line.empty())
            continue;

        boost::iter_split(splits, line, boost::algorithm::first_finder(","));

        if (splits[5].substr(0, 2) != "AM")
            continue;

        auto x = std::stoi(splits[2]);
        auto y = num_cols - 1 - std::stoi(splits[3]);
        auto pid = y * num_cols + x;
        auto channel = splits[5] == "AM1" ? 0 : 1;

        x_map.emplace(x, 0);
        y_map.emplace(y, 0);
        pid_map.emplace(pid, std::vector<int>{channel, x, y});
    }

    auto count = 0;
    for (auto &x : x_map)
        x.second = count++;

    count = 0;
    for (auto &y : y_map)
        y.second = count++;

    count = 0;
    auto cenfiles = std::vector<std::string>();
    auto cen_names = std::vector<std::string>();
    file_in = std::ifstream(cen_files);

    while (std::getline(file_in, line))
    {
        if (line.empty())
            continue;

        cenfiles.emplace_back(line);
        cen_names.emplace_back(get_sample_name(line));
    }

    cpt::format::Cube<double> cen_cube(cad.num_probes(), cenfiles.size(), 2);
    cen_cube.y_axis.set_labels(cen_names);

    for (int s = 0; s < cenfiles.size(); ++s)
    {
        auto cen_data = cpt::format::cen::File::load(cenfiles[s], order);
        auto channel_data = std::vector<std::vector<float>>(2);

        for (int c = 0; c < 2; ++c)
        {
            channel_data[c] = cen_data->extract_intensities(c);

            for (size_t p = 0; p < channel_data[c].size(); ++p)
            {
                cen_cube(p, s, c) = channel_data[c][p];
            }
        }
    }

    auto sketch = std::vector<double>();
    cpt::algorithm::QuantileNormalizationForCube<double> quantile(cen_cube, sketch, 0, 80);

    quantile.target_sketch_estimate();
    quantile.do_quantile_normalization();

    auto qtl_out = std::ofstream();
    auto out_qtl = std::vector<std::string>{};

    for (int s = 0; s < cen_names.size(); ++s)
    {
        for( auto& probe: pid_map)
        {
            auto& pid = probe.first;
            auto& channel = probe.second[0];
            auto& x = probe.second[1];
            auto& y = probe.second[2];

            auto str_data = std::to_string(x_map[x]) + "\t" + std::to_string(y_map[y]) + "\t" + std::to_string(channel);

            for (int c = 0; c < 2; ++c)
                str_data += "\t" + std::to_string(cen_cube(pid, s, c));

            out_qtl.emplace_back(str_data);
        }

        qtl_out = std::ofstream(cen_names[s] + "_qtl.txt");
        qtl_out << "X\tY\tChannels\tChannel0\tChannel1" << std::endl;

        for (auto &str_data : out_qtl)
            qtl_out << str_data << std::endl;

        qtl_out.close();
        out_qtl.clear();
    }

    return 0;
}