#pragma once
#include <CPT/improc/background_fix/sub_and_division01.hpp>
#include <CPT/improc/background_fix/only_sub02.hpp>
namespace cpt { namespace application { namespace intensities_dump { 
struct BackgroundProcess
{
    using This = BackgroundProcess;
    struct Parameters
    {
        std::size_t segment_x_num, segment_y_num;
        float background_percentage;
        std::vector< std::vector<float> > bg_vals;
        std::string algorithm;

        template<class BUF, class JSON>
        static void run( BUF&& buf, JSON&& json )
        {
            auto&& opts = json.get_child( "background_process" );
            buf.This::Parameters::config( opts );
        }
        template<class JSON>
        void config( JSON&& opts )
        {
            segment_x_num           = opts.template get<int32_t>(  "segment_x_num"          );
            segment_y_num           = opts.template get<int32_t>(  "segment_y_num"          );
            background_percentage   = opts
                .template get_optional<float>       (  "background_percentage"  )
                .value_or( 0.02 );
            algorithm               = opts
                .template get_optional<std::string> (  "algorithm"          )
                .value_or( "partial_probe_grid_sub_base02" );
        }
    };
    
    template < class BUFFER >
    static void run( BUFFER&& buf )
    {
        if ( buf.Parameters::algorithm == "partial_probe_grid_sub_base01" )
        {
            buf.Parameters::bg_vals.push_back (
                cpt::improc::background_fix::partial_probe_grid_sub_base01(
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
        else if ( buf.Parameters::algorithm == "partial_probe_grid_sub_base02" )
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
        else
        {
            cpt::warn << "algorithm: " << buf.Parameters::algorithm << " not defined." << std::endl;
            cpt::warn << "use default algorithm: partial_probe_grid_sub_base02" << std::endl;
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

} /* intensities_dump */ 
} /* application */ 
} /* cpt */ 
