#pragma once
#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>
#include <typeindex>
#include <memory>
#include <boost/range.hpp>
#include "celfile.hpp"
// extern "C"
// {
  #include <hdf5.h>
//   #include <hdf5_hl.h>
// }
// #include <Nucleona/language/dll_config.hpp>
namespace cpt{ namespace application{ namespace affy2hdf5{ 
void file_convert_to( 
      const std::string& affycel
    , const std::string& cenhdf5
)
{
    affy::CelData<59> cel;
    cel.read(affycel);
    std::cerr << "to hdf5\n";
    cel.export_hdf5(cenhdf5);
}
}}}
