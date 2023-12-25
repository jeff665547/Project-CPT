#pragma once
#include <CPT/application/cenfile_builder/hdf5_builder.hpp>
// #include <CPT/dll_config.hpp>
namespace cpt{ namespace application{ namespace cenfile_builder{
void build(
      const std::string& schema_json
    , const std::string& result_cenfile_hdf5 
)
{
    bpt::ptree ptree;
    bpt::read_json  ( schema_json, ptree         );
    h5::File file   ( result_cenfile_hdf5, H5F_ACC_TRUNC );
    h5::Group group = file.open_group("/");
    HDF5Builder::build(ptree, group);
}
}}}
