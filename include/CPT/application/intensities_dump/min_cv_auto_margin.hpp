#pragma once
#ifdef CPT__INTENSITIES_DUMP__SEGMENTATION
static_assert(false, "auto_margin.hpp must include before segmentation.hpp");
#endif
#include <CPT/improc/min_cv_auto_margin.hpp>
#include <cstdint>
namespace cpt{ namespace application{ namespace intensities_dump{

struct MinCVAutoMargin
{
    using This = MinCVAutoMargin;
    struct Parameters
    {
        int32_t windows_width, windows_height;
        cv::Mat_<float> cv_mat;                         // all cv value 
        // std::vector<cv::Rect> margin_tiles;
        std::vector<cv::Mat_<int32_t>> detail_raw_values;

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child( "auto_margin" );
            buf.This::Parameters::config( opts );
        }
        template<class JSON>
        void config( JSON&& opts )
        {
            windows_width  = opts.template get<int32_t>( "windows_width"    );
            windows_height = opts.template get<int32_t>( "windows_height"   );
        }
    };
    template < class BUFFER >
    static void run( BUFFER&& buf )
    {
        cpt::improc::MinCVAutoMargin am;
        auto cell_values = am( 
              buf.src
            , buf.tiles
            , buf.windows_width
            , buf.windows_height
        ); 
        cv::Mat_<float> mean  (cv::Size(0, 0), CV_32FC1);
        cv::Mat_<float> stddev(cv::Size(0, 0), CV_32FC1);
        cv::Mat_<float> pixels(cv::Size(0, 0), CV_16UC1);
        for ( auto&& c : cell_values )
        {
            buf.cv_mat.push_back( c.cv_value );
            mean.push_back( c.mean );
            stddev.push_back( c.stddev );
            pixels.push_back( c.detail_raw_value.cols * c.detail_raw_value.rows );
            buf.detail_raw_values.push_back( c.detail_raw_value );
        }
        buf.cv_mat = buf.cv_mat.reshape( 1, buf.feature_rows );
        buf.mean   = std::move(mean  .reshape(1, buf.feature_rows));
        buf.stddev = std::move(stddev.reshape(1, buf.feature_rows));
        buf.pixels = std::move(pixels.reshape(1, buf.feature_rows));
        // buf.margin_tiles = buf.tiles;
    }
};
#define CPT__INTENSITIES_DUMP__AUTO_MARGIN

}}}
