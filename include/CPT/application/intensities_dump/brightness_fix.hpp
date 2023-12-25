#pragma once
// #include <opencv2/core/core.hpp>
// #include <opencv2/imgproc/imgproc.hpp>
// #include <CPT/application/intensities_dump/utility.hpp>
// #include <CPT/utility/mutable.hpp>
#include <CPT/improc/brightness_fix.hpp>
#include <iostream>

namespace cpt { namespace application {
namespace intensities_dump {
struct BrightnessFix : public cpt::improc::BrightnessFix
{
    struct Parameters
    {
        bool enable;
        template<class BUF, class JSON>
        static void run( BUF&& buf, JSON&& json )
        {
            auto&& opt = json.get_child("brightness_fix");
            buf.Parameters::config( opt );
        }
        template<class JSON>
        void config( JSON&& opt )
        {
            this->enable = opt.template get_optional<bool>("enable")
                .value_or(true);
        }
    };
    // template<class T>
    // static auto generate_mean_filter( const cv::Mat_<T>& m, uint16_t rate = 6 )
    // {
    //     auto rows = m.rows * rate / 10;
    //     auto cols = m.cols * rate / 10;
    //     cv::Mat_<float> res( rows, cols, 1.0 / ( rows * cols) );
    //     return res;
    // }
    // template<class T, class FUNC>
    // static auto slide_window( cv::Mat_<T>& grid, uint32_t wh, uint32_t ww, FUNC&& f )
    // {
    //     for ( decltype(grid.rows) i(0); i < grid.rows - wh; i ++ )
    //     {
    //         for ( decltype(grid.cols) j(0); j < grid.cols - ww; j ++ )
    //         {
    //             f( grid, i, j );
    //         }
    //     }
    // }
    // template<class T, class FUNC>
    // static auto segment_window( cv::Mat_<T>& grid, uint32_t wh, uint32_t ww, FUNC&& f )
    // {
    //     for ( decltype(grid.rows) i(0); i < grid.rows; i += wh )
    //     {
    //         for ( decltype(grid.cols) j(0); j < grid.cols; j += ww )
    //         {
    //             f( grid, i, j );
    //         }
    //     }
    // }
    // template < class I, class J, class WH, class WW, class GRID, class MIMG >
    // static auto use_grid_img_black(const I& i, const J& j, const WH& wh, const WW& ww, const GRID& g, const MIMG& mimg)
    // {
    //     cv::Mat_<uint8_t> bw;
    //     cv::normalize( g.clone(), bw, 0, 255.0, cv::NORM_MINMAX, bw.depth() );
    //     cv::threshold( bw, bw, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU );
    //     float back_value(0);
    //     uint32_t back_num(0);
    //     for ( auto ii(i); ii < i + wh && ii < g.rows; ii ++ )
    //     {
    //         for ( auto jj(j); jj < j + ww && jj < g.cols; jj ++ )
    //         {
    //             if ( bw(ii, jj) == 0 )
    //             {
    //                 back_value = ( ( back_value * back_num ) + g( ii, jj ) ) / ( back_num + 1 );
    //                 back_num ++;
    //             }
    //         }
    //     }
    //     for ( auto ii(i); ii < i + wh && ii < g.rows; ii ++ )
    //     {
    //         for ( auto jj(j); jj < j + ww && jj < g.cols; jj ++ )
    //         {
    //             int32_t it = mimg.storage( ii, jj );
    //             int32_t m = back_value;
    //             auto v = it - m;
    //             mimg.storage( ii, jj ) = v >= 0 ? v : 0;
    //             // assert( mimg.storage( ii, jj ) < 60000 );
    //         }
    //     }
    //     // std::cout << mimg.storage << std::endl;
    // }
    // 
    // template< class I, class J, class WH, class WW, class GRID, class MIMG >
    // static auto use_img_bw ( const I& i, const J& j, const WH& wh, const WW& ww, const GRID& g, const MIMG& mimg )
    // {
    //     
    // }

    // template<class I, class G>
    // static auto run_impl( cv::Mat_<I>& img, cv::Mat_<G>& grid )
    // {
    //     // show ( 0, [&]()
    //     // {
    //     //     auto tmp ( bw.clone() );
    //     //     return tmp;
    //     // });
    //     // show ( 0, [&]()
    //     // {
    //     //     auto tmp ( img.clone() );
    //     //     trim_outlier( tmp, 0, 0.02 );
    //     //     cv::normalize( tmp, tmp , 0, 65535, cv::NORM_MINMAX );
    //     //     return tmp;
    //     // });
    //     auto wh = grid.rows * 1 / 20;
    //     auto ww = grid.cols * 1 / 20;
    //     segment_window( 
    //           grid
    //         , wh
    //         , ww
    //         , [&, mimg = cpt::utility::mms(img)]( auto& g, const auto& i, const auto& j )
    //         {
    //             use_grid_img_black( i, j, wh, ww, g, mimg );
    //         }
    //     );
    //     // show ( 0, [&]()
    //     // {
    //     //     auto tmp ( img.clone() );
    //     //     trim_outlier( tmp, 0, 0.02 );
    //     //     cv::normalize( tmp, tmp , 0, 65535, cv::NORM_MINMAX );
    //     //     return tmp;
    //     // });
    // }
    template<class BUF>
    static auto run( BUF&& buf )
    {
        // std::cout << "==========bf=========" << std::endl;
        // std::cout << buf.src(cv::Rect(
        //     0, 0, 50, 50 ) ) << std::endl;
        BrightnessFix bf;
        bf( 
              (cv::Mat_<uint16_t>&)buf.src
            , (cv::Mat_<uint16_t>&)buf.grid_img 
            , buf.Parameters::enable
        );
        // if ( buf.Parameters::enable )
        //     run_impl( 
        //           (cv::Mat_<uint16_t>&)buf.src
        //         , (cv::Mat_<uint16_t>&)buf.grid_img 
        //     );
    }
};
}
}}
