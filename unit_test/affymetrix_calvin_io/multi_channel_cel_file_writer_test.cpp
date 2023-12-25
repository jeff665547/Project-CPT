#include <affymetrix_calvin_io/multi_channel_cel_file_writer.hpp>
#include <affy/calvin_files/parsers/src/CelFileReader.h>
#include <boost/filesystem.hpp>
#include <affy/calvin_files/data/src/GenericDataTypes.h>
#include <affy/calvin_files/parameter/src/ParameterNameValueType.h>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>

TEST(multi_channel_cel_file_writer, basic_test)
{
    // FIXME: The CelFileData may have some memory overflow issues on windows platform
    auto cel_file_name = "./multi_channel_cel_file_writer_test.cel";
    const std::vector<float> int_data{10,20,30,40};
    {
        affymetrix_calvin_io::CelFileData cel_data(cel_file_name);
        auto* p_cel_file_header = cel_data.GetFileHeader(); 
        auto* p_cel_data_header = p_cel_file_header->GetGenericDataHdr();
        affymetrix_calvin_io::GenericDataHeader gdh;
        gdh.SetFileTypeId(MULTI_SCAN_ACQUISITION_DATA_TYPE);
        affymetrix_calvin_parameter::ParameterNameValueType pnv;
        pnv.SetName(L"affymetrix-channel-wavelength");
        pnv.SetValueText(L"0;1",3);
        gdh.AddNameValParam(pnv);
        p_cel_data_header->AddParent(gdh);

        affymetrix_calvin_io::DataGroupHeader dgh;
        dgh.SetName(L"channel-1");
        affymetrix_calvin_io::DataSetHeader dsh;
        dsh.SetRowCnt(4);
        dsh.AddFloatColumn(L"");
        dsh.SetName(CelIntensityLabel);
        dgh.AddDataSetHdr(dsh);
        p_cel_file_header->AddDataGroupHdr( dgh );
        auto& dgh0 = p_cel_file_header->GetDataGroup(0);
        dgh0.SetName(L"channel-0");
        dgh0.AddDataSetHdr(dsh);
        EXPECT_EQ( cel_data.GetChannels().size(), 2 );

        affymetrix_calvin_io::MultiChannelCelFileWriter cel_file_writer(cel_data);
        cel_file_writer.WriteIntensities(0, int_data);
        cel_file_writer.WriteIntensities(1, int_data);
    }
    {
        affymetrix_calvin_io::CelFileData celdata;
        affymetrix_calvin_io::CelFileReader reader;
        reader.SetFilename( cel_file_name );
        reader.Read( celdata );
        celdata.SetActiveChannel(L"channel-0");
        std::vector<float> data;
        celdata.GetIntensities(0, 4, data );
        for ( std::vector<float>::size_type i (0); i < int_data.size(); i ++ )
        {
            std::cout << data.at(i) << std::endl;
            EXPECT_EQ( data.at(i), int_data.at(i) );
        }
        celdata.SetActiveChannel(L"channel-1");
        std::vector<float> data1;
        celdata.GetIntensities(0, 4, data1 );
        for ( std::vector<float>::size_type i (0); i < int_data.size(); i ++ )
        {
            std::cout << data1.at(i) << std::endl;
            EXPECT_EQ( data1.at(i), int_data.at(i) );
        }
        // celdata.Clear();
    }
    // boost::filesystem::remove(cel_file_name); 
}
