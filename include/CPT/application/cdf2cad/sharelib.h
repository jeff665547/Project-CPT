/**
 * @file cdf2cad/sharelib.h
 * @author Chia-Hua Chang
 * @brief Define conversion function of CDF ( Affymetrix Chip Description File ) to CAD format file. 
 */
#pragma once 
#include <string>
namespace cpt { namespace application { 
namespace cdf2cad {
/**
 * @brief Conversion CDF format to CAD format.
 * @details Given the input CDF, output CAD file path, probe table (.probe_tab) and annotation file (.annot.csv), and the function will convert the CDF which is Affymetrix spec to CAD which is Centrillion spec.
 * The Affymetrix file format description can be found at: 
 * http://media.affymetrix.com/support/developer/powertools/changelog/FILE-FORMATS.html
 * Note that the CDF is Axiom platform spec.
 *
 * @param input The input (.cdf) file path.
 * @param output The output (.cad) file path.
 * @param probe_tab The probe table (.prob_tab) file path.
 * @param annot_csv The annotation (.annot.csv) file path.
 * @return none
 *
 * @snippet cdf2cad/sharelib_test.cpp usage
 */
void file_convert_to(
      const std::string& input    
    , const std::string& output   
    , const std::string& probe_tab
    , const std::string& annot_csv
);
}}}
