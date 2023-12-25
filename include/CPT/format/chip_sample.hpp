#pragma once
#include <CFU/format/chip_sample/array.hpp>
#include <CFU/format/chip_sample/array_reader/intensity_list.hpp>
#include <CFU/format/chip_sample/array_reader/probe_grid.hpp>
namespace cpt::format {

using IntensityList = cfu::format::IntensityList; 
using ProbeGrid     = cfu::format::ProbeGrid; 

}
namespace cpt::format::chip_sample {

using Array = cfu::format::chip_sample::Array;

template<class T>
using ArrayReader = cfu::format::chip_sample::ArrayReader<T>;

using IData = cfu::format::chip_sample::IData;
}