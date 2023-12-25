#include <iostream>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>
#include <Nucleona/format/hdf5.hpp>
#include <CPT/application/cenfile_builder/shared.hpp>
extern "C"
{
#include "hdf5.h"
// #include "hdf5_hl.h"
}

namespace bpt = boost::property_tree;
namespace bfs = boost::filesystem;
namespace h5  = nucleona::format::hdf5;


void print(const bpt::ptree& parent)
{
    for (auto& child: parent)
    {
        std::cerr << child.first << '\n';
        print(child.second);
    }
}

int main(int argc, char* argv[])
{
    if( argc <= 2 ) 
    {
        std::cout << "hdf5_schema_builder [input template schema] [output template file]" << std::endl;
        std::cout << "This program is used to create hdf5(CEN) format file." << std::endl;
        std::cout << "It needs a text template file ( in JSON format ) to specify the hdf5 spec." << std::endl;
        exit(1);
    }
    else
    {
        cpt::application::cenfile_builder::build(
            argv[1], argv[2]
        );
        return 0;
    }
}
