#pragma once
#include <utility>
#include <CPT/improc/filters/mat.hpp>
#include <CPT/improc/filters/exclude.hpp>
#include <CPT/improc/filters/helper.hpp>
namespace cpt{ namespace improc{ namespace filter{

constexpr struct RegularMarker
{
    template<class M>
    auto operator()( 
          M&& mat
        , std::size_t marker_width
        , std::size_t marker_height
        , std::size_t marker_x_interval
        , std::size_t marker_y_interval
    ) const
    {
        std::vector<cv::Rect> mask;
        for ( std::size_t i (0); i < detail::rows(mat); i += marker_y_interval )
        {
            for( std::size_t j(0); j < detail::cols(mat); j += marker_x_interval )
            {
                mask.push_back( 
                    cv::Rect( 
                        j, i, marker_width, marker_height
                    )
                );
            }
        }
        return exclude( FWD(mat), mask );
    }
} regular_marker;

}}}
