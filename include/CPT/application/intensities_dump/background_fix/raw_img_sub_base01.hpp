#pragma once
#include <CPT/improc/background_fix/sub_base01.hpp>

namespace cpt{ namespace application{ namespace intensities_dump{ namespace background_fix{
struct RawImgSubBase01
{
    using This = RawImgSubBase01;
    struct Parameters
    {
        std::size_t segment_x_num, segment_y_num;
        float background_percentage;
        bool enable;
        template<class BUF, class JSON>
        static void run( BUF&& buf, JSON&& json )
        {
            auto&& opts = json.get_child( "raw_image_sub_base01" );
            buf.This::Parameters::config( opts );
        }
        template<class JSON>
        void config( JSON&& opts )
        {
            segment_x_num= opts.template get<int32_t>(  "segment_x_num"   );
            segment_y_num= opts.template get<int32_t>(  "segment_y_num"   );
            background_percentage   = opts
                .template get_optional<float>  (  "background_percentage"  )
                .value_or( 0.02 );
            enable= opts.template get<bool>(  "enable"   );
        }
    };
    
    template < class BUFFER >
    static void run( BUFFER& buf )
    {
        if ( buf.Parameters::enable )
        {
            cpt::improc::background_fix::raw_image_sub_base01(
                  (cv::Mat_<uint16_t>&)buf.src
                , buf.tiles
                , buf.markers.at(0).cols
                , buf.markers.at(0).rows
                , buf.marker_x_interval
                , buf.marker_y_interval
                , buf.segment_x_num
                , buf.segment_y_num
                , buf.background_percentage
            );
        }
    }
};
}}}}
