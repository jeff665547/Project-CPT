/**
 * @file cdf2cad_test.cpp
 * @author Chia-Hua Chang
 */
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/application/cdf2cad/sharelib.h>
#include <CPT/application/cadtool/shared.hpp>
#include <Nucleona/test/data_dir.hpp>

using namespace std::string_literals;
#define EXAMPLE_ROOT (nucleona::test::data_dir().string())
TEST( cdf2cad_test, cdf2cad_test )
{
    std::string example_data_dir ( EXAMPLE_ROOT );
    std::string example_out_dir = EXAMPLE_ROOT + "/out/"s;
    if ( !boost::filesystem::exists( example_out_dir ) )
    {
        boost::filesystem::create_directory( example_out_dir );
    }
/// [usage]
    cpt::application::cdf2cad::file_convert_to(
          example_data_dir + "/example.cdf"
        , example_out_dir  + "/cdf2cad_test.cad"
        , example_data_dir + "/example.probe_tab"
        , example_data_dir + "/example.annot.csv"
    );
/// [usage]
    cpt::application::cadtool::cad2json_file_convert_to(
          example_out_dir + "/cdf2cad_test.cad"
        , example_out_dir + "/cdf2cad_test.json"
    );
    // TODO auto check
}
