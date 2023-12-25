/**
 * @file affy2hdf5/sharelib.h
 * @author Chia-Hua Chang
 * @brief Define Affymetrix chip sample file (.cel) to Centrillion chip sample file (.cen) convert function.
 */
#pragma once
#include <string>
namespace cpt{ namespace application{ namespace affy2hdf5{ 
/**
 * @brief Affy CEL file to CEN hdf5 conversion procedure.
 * @details Given the paths to the Affy CEL input file and CEN hdf5 output file and this function will convert the .cel file format to hdf5.
 * @param affycel The .cel file path
 * @param cenhdf5 The .cen file path
 * @snippet affy2hdf5/sharelib_test.cpp usage
 */
void file_convert_to( 
      const std::string& affycel
    , const std::string& cenhdf5
);
}}}
