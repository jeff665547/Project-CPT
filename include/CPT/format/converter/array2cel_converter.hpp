#pragma once
#include <CPT/format/chip_sample.hpp>
#include <affy/calvin_files/writers/src/CalvinCelFileWriter.h>
#include <CPT/logger.hpp>
#include <affy/calvin_files/data/src/GenericDataTypes.h>
namespace cpt{ namespace format{ namespace converter{
namespace cfcs_     = cpt::format::chip_sample;
namespace acu_      = affymetrix_calvin_utilities;
namespace aci_      = affymetrix_calvin_io;
class Array2Cel
{
    auto str2wstr( const std::string& str )
    {
        std::wstring wstr;
        for ( auto&& c : str )
        {
            wstr.push_back( (wchar_t)(c) );
        }
        return wstr;
    }
    auto to_affy_coord_vec ( const std::vector<std::tuple<int16_t, int16_t>>& cpt_coord_vec )
    {
        acu_::XYCoordVector res;
        for ( auto&& o : cpt_coord_vec ) res.emplace_back( std::get<0>(o), std::get<1>(o) );
        return res;
    }
  public:
    auto operator()( const cfcs_::Array& arr, const std::string& cel_file_path )
    {
        std::unique_ptr<aci_::CelFileData> cfd ( new aci_::CelFileData );
        cfd->SetFilename( cel_file_path );
        cfd->GetFileHeader()->GetGenericDataHdr()->SetFileTypeId(INTENSITY_DATA_TYPE);
        cfd->SetCols( arr.feature_columns() );
        cfd->SetRows( arr.feature_rows() );
        cfd->SetArrayType( str2wstr(arr.type()) );
        int i = 0;
        // for ( i = 0; i < arr.channels().size(); i ++ )
        // {
            auto& ch = arr.channels().at(i);
            // std::wstring group_name = (L"channels") + std::to_wstring(i);
            std::wstring group_name = CelDataGroupName;
            aci_::DataGroupHeader dc_hdr( group_name );
            cfd->GetFileHeader()->AddDataGroupHdr(dc_hdr);
            cfd->SetActiveChannel   ( group_name            );
            cfd->SetIntensityCount  ( ch.intensity.size()   ); 
            cfd->SetMaskCount       ( ch.mask.size()        );
            cfd->SetPixelCount      ( ch.pixel.size()       );
            cfd->SetOutlierCount    ( ch.outlier.size()     );
            cfd->SetStdDevCount     ( ch.stddev.size()      );
        // }
        cpt::warn << "Warn : CEL writer temporary not support multi-channel, only first channel will be convert\n" << std::endl;
        aci_::CelFileWriter cfw ( *cfd );
        auto& first_ch = arr.channels().at(0);
        cfw.WriteIntensities    ( first_ch.intensity    );
        cfw.WritePixels         ( first_ch.pixel        );
        cfw.WriteStdDevs        ( first_ch.stddev       );
        cfw.WriteOutlierCoords  ( to_affy_coord_vec ( first_ch.outlier ) );
        cfw.WriteMaskCoords     ( to_affy_coord_vec ( first_ch.mask    ) );
        return true;
    }
};

}}}
