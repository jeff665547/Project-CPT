#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <CPT/utility/mutable.hpp>
namespace cpt { namespace improc
{

struct BrightnessFix
{
    template<class T, class FUNC>
    static auto segment_window( 
        cv::Mat_<T>& grid, uint32_t wh, uint32_t ww, FUNC&& f )
    {
        for ( decltype(grid.rows) i(0); i < grid.rows; i += wh )
        {
            for ( decltype(grid.cols) j(0); j < grid.cols; j += ww )
            {
                f( grid, i, j );
            }
        }
    }
    template < class I, class J, class WH
        , class WW, class GRID, class MIMG >
    static auto use_grid_img_black(
        const I& i, const J& j, const WH& wh, const WW& ww
        , const GRID& g, const MIMG& mimg
    )
    {
        cv::Mat_<uint8_t> bw;
        cv::normalize( g.clone(), bw, 0, 255.0, cv::NORM_MINMAX, bw.depth() );
        cv::threshold( bw, bw, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU );
        float back_value(0);
        uint32_t back_num(0);
        for ( auto ii(i); ii < i + wh && ii < g.rows; ii ++ )
        {
            for ( auto jj(j); jj < j + ww && jj < g.cols; jj ++ )
            {
                if ( bw(ii, jj) == 0 )
                {
                    back_value = ( ( back_value * back_num ) + g( ii, jj ) ) / ( back_num + 1 );
                    back_num ++;
                }
            }
        }
        for ( auto ii(i); ii < i + wh && ii < g.rows; ii ++ )
        {
            for ( auto jj(j); jj < j + ww && jj < g.cols; jj ++ )
            {
                int32_t it = mimg.storage( ii, jj );
                int32_t m = back_value;
                auto v = it - m;
                mimg.storage( ii, jj ) = v >= 0 ? v : 0;
                // assert( mimg.storage( ii, jj ) < 60000 );
            }
        }
        // std::cout << mimg.storage << std::endl;
    }
    template<class I, class G>
    auto operator()( 
          cv::Mat_<I>& img
        , cv::Mat_<G>& grid
        , bool enable
    )
    {
        if ( enable )
        {
            auto wh = grid.rows * 1 / 20;
            auto ww = grid.cols * 1 / 20;
            segment_window( 
                  grid
                , wh
                , ww
                , [&, mimg = cpt::utility::mms(img)]( auto& g, const auto& i, const auto& j )
                {
                    use_grid_img_black( i, j, wh, ww, g, mimg );
                }
            );
        }
    }
};

}}
