#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CCD/analyzer/probe_tm.hpp>

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cout << "./probe_tm_for_anno_csv [anno.csv]" << std::endl;
        return 1;
    }

    int num_rows = 496, num_cols = 496;
    auto anno_file = std::string(argv[1]);

    auto line = std::string();
    auto splits = std::vector<std::string>();

    auto file_in = std::ifstream(anno_file);
    auto outfile = std::ofstream("probe_tm.csv");

    outfile << "pid,x,y,tm" << std::endl;

    while (std::getline(file_in, line))
    {
        if (line.empty() || line.at(0) == ',')
            continue;

        boost::iter_split(splits, line, boost::algorithm::first_finder(","));

        // std::cerr << line << std::endl;
        // std::cerr << splits[2] << "\t" << splits[3] << "\t" << splits[4] << std::endl;

        auto x = std::stoi(splits[2]);
        auto y = num_cols - 1 - std::stoi(splits[3]);
        auto pid = y * num_cols + x;

        probe_tm tm(splits[4]);

        outfile << pid << "," << x << "," << y << "," << tm.tmp << std::endl;
    }

    file_in.close();
    outfile.close();
    return 0;
}