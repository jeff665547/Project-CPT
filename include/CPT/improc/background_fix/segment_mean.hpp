#pragma once
#include <CPT/improc/util.hpp>
#include <CPT/improc/background_fix/index.hpp>
#include <CPT/improc/filters/marker.hpp>
#include <CPT/view/tile.hpp>
#include <CPT/improc/filters/helper.hpp>
#include <CPT/utility/logger.hpp>
namespace cpt{ namespace improc{ namespace background_fix{

struct SegmentMean
{
    constexpr static float floor = 1e-10;

    auto fix_length( std::size_t start, std::size_t expect_length, std::size_t limit ) const 
    {
        auto expect_end = start + expect_length;
        return expect_end > limit ? limit - start : expect_length;
    }
    template<class IMG, class INTEN, class FUNC>
    auto foreach_image_segment(
          INTEN& intens
        , IMG& image
        , std::size_t segment_x_num
        , std::size_t segment_y_num
        , FUNC&& func
    ) const
    {
        namespace cifd = cpt::improc::filter::detail;
        std::vector<float> mean_backs;
        auto seg_height     = ( cpt::improc::filter::detail::rows(intens) + segment_y_num - 1) / segment_y_num;
        auto seg_width      = ( cpt::improc::filter::detail::cols(intens) + segment_x_num - 1 ) / segment_x_num;
        auto rseg_height    = ( cpt::improc::filter::detail::rows(image) + segment_y_num - 1 ) / segment_y_num;
        auto rseg_width     = ( cpt::improc::filter::detail::cols(image) + segment_x_num - 1 ) / segment_x_num;
        
        std::size_t ri = 0;
        for ( std::size_t i = 0; i < cpt::improc::filter::detail::rows(intens); i += seg_height )
        {
            std::size_t rj = 0;
            for ( std::size_t j = 0; j < cpt::improc::filter::detail::cols(intens); j += seg_width )
            {
                auto seg_img_rect  = cv::Rect( 
                      j
                    , i
                    , fix_length( j, seg_width,  cifd::cols(intens) )
                    , fix_length( i, seg_height, cifd::rows(intens) )
                );
                auto rseg_img_rect  = cv::Rect( 
                      rj
                    , ri
                    , fix_length( rj, rseg_width,  cifd::cols( image ) )
                    , fix_length( ri, rseg_height, cifd::rows( image ) )

                );
                mean_backs.push_back( func( 
                      intens
                    , std::move( seg_img_rect  )
                    , image
                    , std::move( rseg_img_rect ) 
                ) );
                cpt::dbg << "seg bg mean (" << i << "," << j << "): " << mean_backs.back() << std::endl;

                rj += rseg_width;
            }
            ri += rseg_height;
        }
        return mean_backs;
    }
    template<class T, class FUNC>
    auto foreach_image_segment(
          T&& mat 
        , std::size_t segment_x_num
        , std::size_t segment_y_num
        , FUNC&& func
    ) const
    {
        namespace cifd = cpt::improc::filter::detail;
        return foreach_image_segment( mat, cifd::unwrap_all( mat ), segment_x_num, segment_y_num, FWD(func) );
    }
};
}}}
