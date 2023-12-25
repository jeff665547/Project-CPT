#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CPT/engine/data_pool/chip_layout.hpp>
#include <CCD/analyzer/probe_tm.hpp>

int main(int argc, const char *argv[])
{
    if (argc != 4)
    {
        std::cout << "./probe_tm_export [cad file] [ps_list] [probe_tm.csv]" << std::endl;
        return 1;
    }

    auto cad_file = std::string(argv[1]);
    auto psl_file = std::string(argv[2]);
    auto ptm_file = std::string(argv[3]);

    cpt::engine::data_pool::GenotypingCadChipLayout cad;
    cad.read_chip_layout(cad_file);

    auto line = std::string();
    auto file_in = std::ifstream(psl_file);

    auto ps_list = std::set<std::string>();
    auto pids = std::set<std::string>();

    while (std::getline(file_in, line))
    {
        if (line.empty() || line == "probeset_id")
            continue;

        ps_list.emplace(line);
    }

    for (int ps = 0; ps < cad.num_probesets(); ++ps)
    {
        if (ps_list.find(cad.probeset_name(ps)) != ps_list.end())
        {
            for (int p = 0; p < cad.num_probes(ps); ++p)
            {
                pids.emplace(std::to_string(cad.probe(ps, p)));
            }
        }
    }
    
    auto splits = std::vector<std::string>();
    
    boost::iter_split(splits, psl_file, boost::algorithm::first_finder("."));

    file_in = std::ifstream(ptm_file);
    auto outfile = std::ofstream("probe_tm_" + splits[0] + ".csv");

    while (std::getline(file_in, line))
    {
        if (line.empty())
            continue;
            
        boost::iter_split(splits, line, boost::algorithm::first_finder(","));

        for( int i = 0; i < splits.size() -1; ++i )
        {
            outfile << splits[i] << ",";
        }

        if (splits[0] == "pid" || pids.find(splits[0]) != pids.end())
        { 
            outfile << splits[splits.size() - 1];
        }
        else
        {
            outfile << "0";
        }

        outfile << std::endl;
    }

    file_in.close();
    outfile.close();
    return 0;
}