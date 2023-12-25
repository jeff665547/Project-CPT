#pragma once
#include <opencv2/opencv.hpp>
#include <CPT/application/intensities_dump/point/point_trait.hpp>

namespace cpt { namespace application {
namespace intensities_dump {

struct Extract
{
    template<class GRID>
    static auto run( GRID&& g )
    {
        return [grid = std::forward<GRID>(g)]( 
                const auto& rect
        )
        {
            return cv::Mat( grid( 
                // make_cv_rect( point_0, point_e )
                rect
            ));
        };
    }
    // template<class X0, class Y0, class XE, class YE>
    // static auto run( 
    //       const cv::Mat& grid
    //     , const X0& x0, const Y0& y0
    //     , const XE& xe, const YE& ye 
    // )
    // {
    //     return run( grid ) ( x0, y0, xe, ye );
    // }
};

}}}

