/**
 *  @file       CPT/improc/background_fix/sub_and_division01.hpp
 *  @author     Chia-Hua Chang
 *  @brief      The background fix algorithm ( number 1 ). 
 *              First, compute the local background and subtracted. 
 *              Second, compute the global background and division.
 */
#pragma once
#include <CPT/improc/util.hpp>
#include <CPT/improc/background_fix/index.hpp>
#include <CPT/improc/filters/marker.hpp>
#include <CPT/view/tile.hpp>
#include <CPT/improc/filters/helper.hpp>
#include <CPT/utility/logger.hpp>
#include <CPT/improc/background_fix/segment_mean.hpp>
namespace cpt{ namespace improc{ namespace background_fix{

/**
 *  @brief      @copybrief CPT/improc/background_fix/sub_and_division.hpp
 *  @details    The detail information can see here @ref improc_background_fix_sub_and_division01. See @ref improc_background_fix_sub_and_division01 for detail.
 */
constexpr struct PartialProbeGridSubAndDivisionBase01 : public SegmentMean
{
    /**
     *  @brief  Local background process of image.
     *  @param  grid                        The probe grid after ROI ( probe domain image )
     *  @param  image                       The raw image after ROI  ( pixel domain image )
     *  @param  marker_width                The width of marker
     *  @param  marker_height               The height of marker
     *  @param  marker_x_interval           The interval between markers in x direction and is in probe domain
     *  @param  marker_y_interval           The interval between markers in y direction and is in probe domain
     *  @param  segment_x_num               The x direction number of local segmentation
     *  @param  segment_y_num               The y direction number of local segmentation
     *  @param  background_trimmed_percent  The percentages of the probes in a single segments, which use to compute the local background
     */
    auto operator()(
          cv::Mat_<float>& grid // intensities
        , cv::Mat& image
        , std::size_t marker_width 
        , std::size_t marker_height
        , std::size_t marker_x_interval
        , std::size_t marker_y_interval
        , std::size_t segment_x_num
        , std::size_t segment_y_num
        , float background_trimmed_percent
    ) const
    {
        namespace cifd = cpt::improc::filter::detail;
        image.convertTo( image, CV_32F );
        auto filter_marker_mat = cpt::improc::filter::regular_marker( 
              grid
            , marker_width
            , marker_height
            , marker_x_interval
            , marker_y_interval 
        );
        cpt::dbg << "image size: " << image.rows << ", " << image.cols << std::endl;
        auto mbs = foreach_image_segment( 
            filter_marker_mat, image, segment_x_num, segment_y_num, 
            [
                  marker_width
                , marker_height
                , marker_x_interval
                , marker_y_interval 
                , background_trimmed_percent
            ]( auto&& intens, auto&&  seg_rect, auto&& img, auto&& img_seg_rect)
            {
                auto&& intens_seg = intens(seg_rect);
                {
                    cv::Mat tmp = cifd::unwrap_all(intens).clone();
                    cv::normalize( tmp, tmp, 0, 65535, cv::NORM_MINMAX, CV_16U );
                    cv::rectangle( tmp, seg_rect, cv::Scalar(30000) );
                    // cv::imwrite( 
                    //     "seg" + std::to_string(seg_rect.x) + "_" + std::to_string(seg_rect.y) + ".tif" 
                    //     , tmp
                    // );

                }
                {
                    cpt::dbg << "image seg point: " << img_seg_rect.x << ", " << img_seg_rect.y << std::endl;
                    cpt::dbg << "image seg size: " << img_seg_rect.height << ", " << img_seg_rect.width << std::endl;
                    cv::Mat tmp = img.clone();
                    cv::normalize( tmp, tmp, 0, 65535, cv::NORM_MINMAX, CV_16U );
                    cv::rectangle( tmp, img_seg_rect, cv::Scalar(30000) );
                    // cv::imwrite( 
                    //     "seg" + std::to_string(img_seg_rect.x) + "_" + std::to_string(img_seg_rect.y) + "_r.tif" 
                    //     , tmp
                    // );

                }
                std::vector<float> means;
                auto total_run_num = intens_seg.foreach([ &means]( auto&& m, auto i, auto j )
                {
                    auto mean = m( i, j );
                    means.push_back( mean );
                });
                std::sort( means.begin(), means.end() );
                std::size_t total_mean_num = [ exp = means.size() * background_trimmed_percent ](){ return exp < 1 ? 1 : exp;}();
                double inst_sum = 0;
                for ( std::size_t i = 0; i < total_mean_num; i ++ )
                {
                    inst_sum += means.at(i);
                }
                auto seg_bg_mean = inst_sum / total_mean_num;
                for( auto i = img_seg_rect.y; i < img_seg_rect.y + img_seg_rect.height; i ++ )
                {
                    for ( auto j = img_seg_rect.x; j < img_seg_rect.x + img_seg_rect.width; j ++ )
                    {
                        auto&& v = img.template at<float>( i, j );
                        auto tmp = (v - seg_bg_mean) / seg_bg_mean;
                        if ( tmp > floor )
                            v = tmp;
                        else
                            v = floor;
                    }
                }
                return seg_bg_mean;
            }
        );
        return mbs;
        // this->probe_grid_proess( FWD(o)... );
    }
} partial_probe_grid_sub_base01;

}}}
