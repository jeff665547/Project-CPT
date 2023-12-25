#pragma once
#include <CPT/improc/background_fix/only_sub02.hpp>
namespace cpt{ namespace application{ namespace intensities_dump{ namespace background_fix{

struct PartialProbeGridSubAndDivisionBase02
{
    using This = PartialProbeGridSubAndDivisionBase02;
    struct Parameters
    {
        std::size_t segment_x_num, segment_y_num;
        float background_percentage;
        std::vector< std::vector<float> > bg_vals;
        bool enable;
        template<class BUF, class JSON>
        static void run( BUF&& buf, JSON&& json )
        {
            auto&& opts = json.get_child( "partial_probe_grid_sub_base02" );
            buf.This::Parameters::config( opts );
        }
        template<class JSON>
        void config( JSON&& opts )
        {
            segment_x_num           = opts.template get<int32_t>(  "segment_x_num"          );
            segment_y_num           = opts.template get<int32_t>(  "segment_y_num"          );
            background_percentage   = opts
                .template get_optional<float>  (  "background_percentage"  )
                .value_or( 0.02 );
            enable= opts.template get<bool>(  "enable"   );
        }
    };
    
    template < class BUFFER >
    static void run( BUFFER&& buf )
    {
        if ( buf.Parameters::enable )
        {
            buf.Parameters::bg_vals.push_back (
                cpt::improc::background_fix::partial_probe_grid_sub_base02(
                      (cv::Mat_<float>&)    buf.mean
                    // , (cv::Mat_<uint16_t>&) buf.src
                    , buf.src
                    , buf.markers.at(0).cols
                    , buf.markers.at(0).rows
                    , buf.marker_x_interval
                    , buf.marker_y_interval
                    , buf.segment_x_num
                    , buf.segment_y_num
                    , buf.background_percentage
                ) 
            )
            ;
        }
    }
};
}}}}
