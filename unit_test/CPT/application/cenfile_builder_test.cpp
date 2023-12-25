/**
 * @file cenfile_builder_test.cpp
 * @author Chia-Hua Chang
 */
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <CPT/application/cenfile_builder/sharelib.h>
#include <string>
#include <boost/filesystem.hpp>
using namespace std::string_literals;
#define EXAMPLE_ROOT (nucleona::test::data_dir().string())
TEST( cenfile_builder_test, cenfile_builder_test )
{
    std::string example_data_dir = EXAMPLE_ROOT;
    std::string example_out_dir = EXAMPLE_ROOT + "/out/"s;
    if ( !boost::filesystem::exists( example_out_dir ) )
    {
        boost::filesystem::create_directory( example_out_dir );
    }
/// [usage]
    cpt::application::cenfile_builder::build(
          example_data_dir + "/hdf5_schema_example.json"
        , example_out_dir + "/GSM2066668_206-001_CHB.CEN"
    );
/// [usage]
}
