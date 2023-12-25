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

    if (argc != 4 && argc != 6)
    {
        std::cout << "ps_viewer [probeset_id] [cad file] [cen files list]" << std::endl;
        std::cout << "ps_viewer [probeset_id] [cad file] [cen files list] [ps_list] [mdl file] " << std::endl;
        return 1;
    }

    std::vector< std::string > order{"Banff_AM1E_red","Banff_AM3_green"};

    auto probeset_id = std::string(argv[1]);
    auto cad_file = std::string(argv[2]);
    auto cen_files = std::string(argv[3]);
    auto ps_list = std::string();
    auto mdl_file = std::string();

    if (argc == 6)
    {
        ps_list = argv[4];
        mdl_file = argv[5];
    }

    auto line = std::string();
    auto ps_idx = std::string();
    auto mdl_line = std::string();
    auto file_in = std::ifstream(ps_list);
    auto count = 0;

    if (argc == 6)
    {
        while (std::getline(file_in, line))
        {
            count++;

            if (line.empty() || line == "probeset_id" || line != probeset_id)
                continue;

            ps_idx = std::to_string(count - 2);
            std::cerr << "Found probeset " << probeset_id << " at index " << ps_idx << " in " << ps_list << std::endl;
        }

        file_in = std::ifstream(mdl_file);
        auto splited = std::vector<std::string>();

        while (std::getline(file_in, line))
        {
            if (line.empty() || line.substr(0, 3) == "Idx")
                continue;

            boost::iter_split(splited, line, boost::algorithm::first_finder("\t"));

            if (splited[0] != ps_idx)
                continue;

            mdl_line = line;
            std::cerr << mdl_line << std::endl;
        }
    }

    cpt::engine::data_pool::GenotypingCadChipLayout cad;
    cad.read_chip_layout(cad_file);

    auto probeset_idx = 0;

    for (int p = 0; p < cad.num_probesets(); ++p)
    {
        if (cad.probeset_name(p) == probeset_id)
        {
            probeset_idx = p;
            std::cerr << "Found probeset " << probeset_id << " at index " << probeset_idx << " in " << cad_file << std::endl;
            break;
        }
    }

    std::cerr << cad.num_probes(probeset_idx) << " probes in probeset " << probeset_id << std::endl;
    for (int p = 0; p < cad.num_probes(probeset_idx); ++p)
        std::cerr << p << ": " << cad.probe(probeset_idx, p) << std::endl;

    count = 0;
    auto cenfiles = std::vector<std::string>();
    auto cen_names = std::vector<std::string>();
    file_in = std::ifstream(cen_files);

    while (std::getline(file_in, line))
    {
        if (line.empty())
            continue;

        // count++;
        // std::cerr << "Loading " << count << ": " << cenfile << "\r";

        cenfiles.emplace_back(line);
        cen_names.emplace_back(get_sample_name(line));
    }

    // std::cerr << std::endl;

    cpt::format::Cube<double> cen_cube(cad.num_probes(), cenfiles.size(), 2);
    cen_cube.y_axis.set_labels(cen_names);

    for (int s = 0; s < cenfiles.size(); ++s)
    {
        // auto cen_data = cpt::format::cen::File::load(cenfiles[s]);
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

    std::set<std::string> out_raw;

    for (int p = 0; p < cad.num_probes(probeset_idx); ++p)
    {
        for (int s = 0; s < cen_names.size(); ++s)
        {
            auto str_data = std::to_string(cad.probe(probeset_idx, p)) + "\t" + cen_names[s];

            for (int c = 0; c < 2; ++c)
                str_data += "\t" + std::to_string(cen_cube(cad.probe(probeset_idx, p), s, c));

            out_raw.emplace(str_data);
        }
    }

    std::ofstream raw_out(probeset_id + "_raw.txt");
    raw_out << "PID\tSample\tChannel0\tChannel1" << std::endl;

    for (auto &str_data : out_raw)
        raw_out << str_data << std::endl;

    raw_out.close();

    auto sketch = std::vector<double>();
    cpt::algorithm::QuantileNormalizationForCube<double> quantile(cen_cube, sketch, 0, 80);

    quantile.target_sketch_estimate();
    quantile.do_quantile_normalization();

    std::map<int, std::vector<double>> channel_a, channel_b;
    std::map<int, std::map<int, double>> channel_pa, channel_pb, channel_pp;

    std::set<std::string> out_qtl;

    for (int p = 0; p < cad.num_probes(probeset_idx); ++p)
    {
        for (int s = 0; s < cen_names.size(); ++s)
        {
            auto str_data = std::to_string(cad.probe(probeset_idx, p)) + "\t" + cen_names[s];

            for (int c = 0; c < 2; ++c)
                str_data += "\t" + std::to_string(cen_cube(cad.probe(probeset_idx, p), s, c));

            out_qtl.emplace(str_data);

            if (cad.probe_allele(probeset_idx, p) == 'A')
            {
                channel_a[s].emplace_back(cen_cube(cad.probe(probeset_idx, p), s, cad.probe_channel(probeset_idx, p)));
                channel_pa[cad.probe(probeset_idx, p)][s] = cen_cube(cad.probe(probeset_idx, p), s, cad.probe_channel(probeset_idx, p));
            }

            if (cad.probe_allele(probeset_idx, p) == 'B')
            {
                channel_b[s].emplace_back(cen_cube(cad.probe(probeset_idx, p), s, cad.probe_channel(probeset_idx, p)));
                channel_pb[cad.probe(probeset_idx, p)][s] = cen_cube(cad.probe(probeset_idx, p), s, cad.probe_channel(probeset_idx, p));
            }
        }
    }

    std::ofstream qtl_out(probeset_id + "_qtl.txt");
    qtl_out << "PID\tSample\tChannel0\tChannel1" << std::endl;

    for (auto &str_data : out_qtl)
        qtl_out << str_data << std::endl;

    qtl_out.close();

    double a = 0.0, b = 0.0;
    auto tr_cube = cpt::format::Cube<double>(cad.num_probes(probeset_idx), cen_names.size(), 2);

    std::ofstream trans_out(probeset_id + "_trans.txt");
    trans_out << "PID\tSample\tChannel0\tChannel1" << std::endl;

    for (int p = 0; p < cad.num_probes(probeset_idx); ++p)
    {
        for (int s = 0; s < cen_names.size(); ++s)
        {
            tr_cube(p, s, 0) = channel_pa[cad.probe(probeset_idx, p)][s];
            tr_cube(p, s, 1) = channel_pb[cad.probe(probeset_idx, p)][s];
        }
    }

    cpt::algorithm::Transformations<double> trans(tr_cube);

    for (int p = 0; p < cad.num_probes(probeset_idx); ++p)
    {
        for (int s = 0; s < cen_names.size(); ++s)
        {
            a = tr_cube(p, s, 0);
            b = tr_cube(p, s, 1);

            trans.log_ratio_strength(a, b);
            trans_out << std::to_string(cad.probe(probeset_idx, p)) << "\t" << cen_names[s] << "\t" << a << "\t" << b << std::endl;
        }
    }

    trans_out.close();
    auto ps_cube = cpt::format::Cube<double>(1, cen_names.size(), 2);

    for (int s = 0; s < cen_names.size(); ++s)
    {
        a = 0.0;
        b = 0.0;

        for (int p = 0; p < channel_a[s].size(); ++p)
            a += channel_a[s][p];

        for (int p = 0; p < channel_b[s].size(); ++p)
            b += channel_b[s][p];

        if (a != 0)
            ps_cube(0, s, 0) = a / channel_a[s].size();

        if (b != 0)
            ps_cube(0, s, 1) = b / channel_b[s].size();
    }

    cpt::algorithm::Transformations<double> summz(ps_cube);

    std::ofstream summz_out(probeset_id + "_summz.txt");
    summz_out << "#" << mdl_line << std::endl;
    summz_out << "Sample\tX\tY" << std::endl;

    for (int s = 0; s < ps_cube.n_cols; ++s)
    {
        a = ps_cube(0, s, 0);
        b = ps_cube(0, s, 1);

        summz.log_ratio_strength(a, b);
        summz_out << cen_names[s] << "\t" << a << "\t" << b << std::endl;
    }

    summz_out.close();
    return 0;
}