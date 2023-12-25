#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>
#include <typeindex>
#include <memory>
#include <boost/range.hpp>
#include <CPT/application/affy2hdf5/celfile.hpp>
// extern "C"
// {
#include <hdf5.h>
//   #include <hdf5_hl.h>
// }
#include <CPT/application/affy2hdf5/shared.hpp>


int main(int argc, char* argv[])
{
    if( argc <= 2 )
    {
        std::cout << "affy2hdf5 [in/output hdf5] [input .cel file]" << std::endl;
        std::cout << "This program is used to convert an Affymetrix CEL file into hdf5(CEN) file.\n";
        std::cout << "The conversion only works in conditions as follows :\n";
        std::cout << "1. The CEL file should be in 'Command Console Version 1' Format defined by Affymetrix, whose magic number is 59.\n";
        std::cout << "2. The input hdf5 (CEN) file should be an uninitialised empty CEN template file, which can be generated directly by 'hdf5_schema_builder'\n";
        std::cout << "   This file will be updated in place with the content from the CEL file.\n";
        exit(1);
    }
    else
    {
        cpt::application::affy2hdf5::file_convert_to( argv[2], argv[1] );
    }
    return 0;
}
