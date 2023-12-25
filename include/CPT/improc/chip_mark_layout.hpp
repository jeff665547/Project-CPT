#pragma once
#include <CPT/improc/util.hpp>
namespace cpt { namespace improc {

template<class BUF>
struct ChipMarkLayout
{
    // TODO : fix to single value, not save all matrix
    
    ChipMarkLayout( cv::Mat_<float>& marker_scores, BUF& buf )
    : layout_scores_( 
          layout_score_rows( marker_scores, buf )
        , layout_score_cols( marker_scores, buf )
    )
    , marker_scores_ ( marker_scores )
    , buf_ ( buf )
    {}

    auto push_score( int r, int c  )
    {
        float sum (0);
        decltype(r) it = buf_.y_marker_num;
        for ( 
            auto i = r; 
            i < marker_scores_.rows
            && it > 0; 
            i += buf_.marker_y_interval 
        )
        {
            decltype(c) jt = buf_.x_marker_num;
            for ( 
                auto j = c; 
                j < marker_scores_.cols
                && jt > 0; 
                j += buf_.marker_x_interval 
            )
            {
                sum += marker_scores_( i, j );
                jt --;
            }
            it --;
        }
        layout_scores_( r, c ) = sum; 

    }
    auto get_max_score_with_pos()
    {
        int mi(0), mj(0);
        for ( int i = 0; i < layout_scores_.rows; i ++ )
        {
            for ( int j = 0; j < layout_scores_.cols; j ++ )
            {
                if ( layout_scores_( i, j ) > layout_scores_( mi, mj ) )
                {
                    mi = i;
                    mj = j;
                }
            }
        }
        struct 
        {
            int i, j;
            float score;
        } res { mi, mj, layout_scores_( mi, mj ) };
        return res;
    }
    template<class T, class M>
    auto get_bound_rect( T&& pos_score, const M& marker )
    {
        auto rbi = pos_score.i + ( ( buf_.y_marker_num - 1 ) * buf_.marker_y_interval ) + marker.rows;
        auto rbj = pos_score.j + ( ( buf_.x_marker_num - 1 ) * buf_.marker_x_interval ) + marker.cols;

        return cv::Rect(
              cv::Point( pos_score.j, pos_score.i )
            , cv::Point( rbj, rbi )
        );
    }
    auto get_layout_scores_n_rows()
    {
        return layout_scores_.rows;
    }
    auto get_layout_scores_n_cols()
    {
        return layout_scores_.cols;
    }
    auto& get_layout_scores()
    {
        return layout_scores_;
    }
  private:
    cv::Mat_<float> layout_scores_;
    cv::Mat_<float>& marker_scores_;
    BUF& buf_;
   
    static auto layout_score_rows ( 
          cv::Mat_<float>& marker_scores
        , BUF& buf
    )
    {
        return marker_scores.rows 
            - ( 
                  ( buf.y_marker_num - 1 ) 
                * buf.marker_y_interval 
            );
    }
    static auto layout_score_cols ( 
          cv::Mat_<float>& marker_scores
        , BUF& buf
    )
    {
        return marker_scores.cols 
            - ( 
                  ( buf.x_marker_num - 1 ) 
                * buf.marker_x_interval 
            );
    }
};

}}
