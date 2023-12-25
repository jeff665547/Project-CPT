/**
 * @file cenfile_builder/sharelib.h
 * @author Chia-Hua Chang
 * @brief Define the JSON schema to CEN HDF5 file building function. 
 */
#pragma once
#include <string>
#include <CPT/application/cenfile_builder/sharelib.h>
namespace cpt{ namespace application{ namespace cenfile_builder{
/**
 * @brief Build the CEN HDF5 file from the JSON schema file.
 * @details This function read a human-readable schema description JSON file and generate the Centrillion sample file (.cen). 
 * Note that the JSON file format should follow this schema. ( http://cgitlab.jhhlab.tw/centrillion/CPT/wikis/DocumentForHDF5Builder ) 
 *
 * @param schema_json the schema description (.json) file path
 * @param result_cenfile_hdf5 the result (.cen) file path
 * @return none
 *
 * @snippet cenfile_builder/sharelib_test.cpp usage
 */
void build(
      const std::string& schema_json
    , const std::string& result_cenfile_hdf5 
);
}}}
