#pragma once
#include <CPT/improc/util.hpp>
#include <CPT/utility/language.hpp>
#include <CPT/improc/filters/helper.hpp>
namespace cpt{ namespace improc{ namespace filter{
template<class T> struct Mat;
template<class T>
Mat<T> make_mat( T&& c, const std::vector<cv::Rect>& mask );
template<class T>
struct Mat
{
    Mat( T&& c, const std::vector<cv::Rect>& mask )
    : core( FWD(c) )
    , masks_( mask )
    {
    }
    bool is_masked( int i, int j )
    {
        for ( cv::Rect& m : masks_ )
        {
            if( 
                   i >= m.y 
                && i < m.y + m.height 
                && j >= m.x
                && j < m.x + m.width
            )
            {
                return true;
            }
        }
        return false;
    }
    template<class FUNC>
    auto foreach( FUNC&& func )
    {
        int total_run_num = 0;
        for ( int i = 0; i < detail::rows(core); i ++ )
        {
            for ( int j = 0; j < detail::cols(core); j ++ )
            {
                if ( !is_masked( i, j ) )
                {
                    total_run_num ++;
                    func(core, i, j );
                }
            }
        }
        return total_run_num;
    }
    auto n_rows()
    {
        return detail::rows(core);
    }
    auto n_cols()
    {
        return detail::cols(core);
    }
    auto operator()(const cv::Rect& r )
    {
        std::vector<cv::Rect> split_masks;
        for ( auto m : masks_ )
        {
            m.x -= r.x;
            m.y -= r.y;
            if ( m.x >= 0 && m.y >= 0 )
                split_masks.emplace_back( std::move(m) );
        }
        return make_mat( core(r), std::move(split_masks));
    }
    decltype(auto) unwrap_all()
    {
        return detail::unwrap_all( core );
    } 
    T core;
  protected:
    std::vector<cv::Rect> masks_;
};
template<class T>
Mat<T> make_mat( T&& c, const std::vector<cv::Rect>& mask )
{
    return Mat<T>( FWD(c), mask );
}


}}}
