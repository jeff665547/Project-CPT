/**
 * @file cadtool_test.cpp
 * @author Chia-Hua Chang
 */
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/application/cadtool/sharelib.h>
#include <CPT/format/json.hpp>
#include <string>
#include <boost/filesystem.hpp>
#include <Nucleona/test/data_dir.hpp>
using namespace std::string_literals;
TEST ( cadtool_test, json_cad_conversion )
{
    std::string example_data_dir ( nucleona::test::data_dir().string() );
    std::string example_out_dir = example_data_dir + "/output/"s;
    if ( !boost::filesystem::exists( example_out_dir ) )
    {
        boost::filesystem::create_directory( example_out_dir );
    }
/// [json2cad]
    cpt::application::cadtool::json2cad_file_convert_to( 
          example_data_dir + "/cad_example.json"
        , example_out_dir + "/json2cad_test.cad" 
    );
/// [json2cad]
/// [cad2json]
    cpt::application::cadtool::cad2json_file_convert_to( 
          example_out_dir + "/json2cad_test.cad"
        , example_out_dir +  "/cad2json_test.json" 
    );
/// [cad2json]
/// [general]
    cpt::application::cadtool::file_convert_to(
          example_out_dir + "/json2cad_test.cad"
        , example_out_dir +  "/cad2json_test2.json"
        , "cad2json"
    );
    cpt::application::cadtool::file_convert_to(
          example_out_dir +  "/cad2json_test2.json"
        , example_out_dir + "/json2cad_test2.cad"
        , "json2cad"
    );
/// [general]
}

