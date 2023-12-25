/**
 * @file affy2hdf5_test.cpp
 * @author Chia-Hua Chang
 */
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/application/affy2hdf5/sharelib.h>
#include <CPT/application/cenfile_builder/sharelib.h>
#include <string>
#include <boost/filesystem.hpp>
#include <Nucleona/test/data_dir.hpp>

using namespace std::string_literals;
#define EXAMPLE_ROOT (nucleona::test::data_dir().string())
TEST( affy2hdf5_test, affy2hdf5_test )
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
    cpt::application::affy2hdf5::file_convert_to(
          example_data_dir + "/GSM2066668_206-001_CHB.CEL"
        , example_out_dir + "/GSM2066668_206-001_CHB.CEN"
    );
/// [usage]
}
