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
        std::cout << "./np_probe_export [np.ps] [cad file] [cen files list]" << std::endl;
        return 1;
    }

    std::vector<std::string> order{"Banff_AM1E_red", "Banff_AM3_green"};

    auto np_file = std::string(argv[1]);
    auto cad_file = std::string(argv[2]);
    auto cen_files = std::string(argv[3]);

    auto line = std::string();
    auto file_in = std::ifstream(np_file);
    auto np_pids = std::set<std::string>();

    while (std::getline(file_in, line))
    {
        if (line.empty() || line == "probeset_id")
            continue;
        np_pids.emplace(line);
    }

    cpt::engine::data_pool::GenotypingCadChipLayout cad;
    cad.read_chip_layout(cad_file);

    auto np_pidx = std::vector<int>();

    for (int p = 0; p < cad.num_probesets(); ++p)
    {
        if (np_pids.find(cad.probeset_name(p)) != np_pids.end())
            np_pidx.emplace_back(p);
    }

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
        for (auto &np_idx : np_pidx)
        {
            for (int p = 0; p < cad.num_probes(np_idx); ++p)
            {
                std::string str_data = cad.probe_allele(np_idx, p) == 'A' ? "AT" : "CG";

                for (int c = 0; c < 2; ++c)
                    str_data += "\t" + std::to_string(cen_cube(cad.probe(np_idx, p), s, c));

                out_qtl.emplace_back(str_data);
            }
        }

        qtl_out = std::ofstream(cen_names[s] + "_qtl.txt");
        qtl_out << "AT/CG\tChannel0\tChannel1" << std::endl;

        for (auto &str_data : out_qtl)
            qtl_out << str_data << std::endl;

        qtl_out.close();
        out_qtl.clear();
    }

    return 0;
}