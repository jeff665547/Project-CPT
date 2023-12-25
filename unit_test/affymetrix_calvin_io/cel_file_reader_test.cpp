#include <affy/calvin_files/parsers/src/CelFileReader.h>
#include <CPT/logger.hpp>
#include <iostream>
#include <string>
#include <CPT/format/converter/wstring_conv.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
using namespace std::string_literals;
TEST(cel_file_reader, multi_channel_test)
{
    // FIXME: The CelFileData may have some memory overflow issues on windows platform
    cpt::format::WStrUTF8* p_code_conv = new cpt::format::WStrUTF8();
    auto& code_conv = *p_code_conv;
    affymetrix_calvin_io::CelFileData cel_data;
    affymetrix_calvin_io::CelFileReader cel_file_reader;

    cel_file_reader.SetFilename( (nucleona::test::data_dir() / "GSM2066668_206-001_CHB.CEL").string() );
    cel_file_reader.Read( cel_data );
    auto channel_names( cel_data.GetChannels() );
    for ( auto& name : channel_names )
    {
        cpt::dbg << code_conv.to_bytes(name) << std::endl;
        cel_data.SetActiveChannel( name );
        std::vector<float> intensities;
        EXPECT_EQ( cel_data.GetIntensities(0, 10, intensities ), true );
        for ( auto& v : intensities )
        {
            cpt::dbg << v << '\t';
        } 
        cpt::dbg << std::endl;
    }
    delete p_code_conv;
}
