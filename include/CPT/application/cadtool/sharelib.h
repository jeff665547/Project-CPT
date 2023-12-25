/**
 * @file cadtool/sharelib.h
 * @author Chia-Hua Chang
 * @brief Define the conversion method between binary CAD file and human-readable JSON text format.
 */
#pragma once
#include <string>
namespace cpt{ namespace application{
namespace cadtool{ 

/**
 * @brief Do the file conversion from JSON to CAD.
 * @details Given the input JSON file path and output CAD file path, This function will convert the JSON file which is the human-readable text file to the CAD binary data. 
 * Note that the JSON file format should follow this schema ( http://cgitlab.jhhlab.tw/centrillion/CPT/wikis/CAD-format-spec ). 
 *
 * @param input_file_path The input (.json) file path.
 * @param output_file_path The output (.cad) file path.
 * @return none
 *
 * @snippet cadtool/sharelib_test.cpp json2cad
 */
void json2cad_file_convert_to(
      const std::string& input_file_path
    , const std::string& output_file_path  
);
/**
 * @brief Define the format conversion method from CAD to JSON.
 * @details Given the input CAD file path and output JSON file path. This function will convert the CAD file which is binary data to the human-readable JSON file. 
 * Note that the JSON file format will follow this schema ( http://cgitlab.jhhlab.tw/centrillion/CPT/wikis/CAD-format-spec ). 
 *
 * @param input_file_path The input .cad file path
 * @param output_file_path The output .json file path
 * @return none
 *
 * @snippet cadtool/sharelib_test.cpp cad2json
 */
void cad2json_file_convert_to(
      const std::string& input_file_path
    , const std::string& output_file_path  
);
/**
 * @brief Do the file conversion from JSON to CAD.
 * @details This function defined a wrapper of 2 functions, json2cad_file_convert_to and cad2json_file_convert_to.
 * It provides a string parameter to switch the user required conversion. 
 * @param input_file_path The input (.json) file path
 * @param output_file_path The output (.cad) file path
 * @param mode The string used to swtich internal function, 
 * can be json2cad or cad2json
 * @return none
 *
 * @snippet cadtool/sharelib_test.cpp general
 */
void file_convert_to(
      const std::string& input_file_path
    , const std::string& output_file_path  
    , const std::string& mode
);

}}}
