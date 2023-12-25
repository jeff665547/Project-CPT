#pragma once
#include <CPT/improc/util.hpp>

namespace cpt { namespace improc {
struct ROIQC
{
    struct PointLess
    {
        bool operator()( const cv::Point& p1, const cv::Point& p2 )
        {
            if ( p1.x == p2.x )
                return p1.y < p2.y;
            else
                return p1.x < p2.x;
        }
    };
    template<class T>
    auto max_pos( const cv::Mat_<T>& m )
    {
        cv::Point res(0, 0);
        for ( int i = 0; i < m.rows; i ++ )
        {
            for ( int j = 0; j < m.cols; j ++ )
            {
                if( m(i,j) > m(res.y, res.x) )
                {
                    res.x = j;
                    res.y = i;
                }
            }
        }
        return res;
    }
    auto operator()(
          const cv::Mat_<float>& score
        , const cv::Point& max_point
        , const uint32_t& test_width
        , const uint32_t& test_height
        , const uint32_t& x_marker_num
        , const uint32_t& y_marker_num
        , const uint32_t& marker_x_interval
        , const uint32_t& marker_y_interval
    )
    {
        std::map<cv::Point, int, PointLess> check_tab;
        for( uint32_t i = 0; i < y_marker_num; i ++ )
        {
            for( uint32_t j = 0; j < x_marker_num; j ++ )
            {
                auto test_pos_x = j * marker_x_interval;
                auto test_pos_y = i * marker_y_interval;

                cv::Rect test_range(
                      cv::Point( test_pos_x, test_pos_y )
                    , cv::Point( 
                          test_pos_x + test_width
                        , test_pos_y + test_height
                    )
                );
                auto test_mat = score(test_range);
                auto max_loc = max_pos( test_mat );
                auto itr = check_tab.find( max_loc );
                if ( itr == check_tab.end() ) 
                    check_tab.emplace( max_loc, 0 );
                else
                    itr->second ++;
            }
        } 
        for ( auto&& p : check_tab )
        {
            if ( p.second >= 2 )
                return true;
        }
        return false;
    } 
};

}}
