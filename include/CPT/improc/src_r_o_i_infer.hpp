/**
 * @file    CPT/improc/src_r_o_i_infer.hpp
 * @author  Chia-Hua Chang 
 * @brief   Apply the ROI result to pixel domain data (image, gridline, tiles etc.)
 */
#pragma once
#include <CPT/improc/util.hpp>
#include <CPT/improc/chip_mark_layout.hpp>
#include <CPT/view.hpp>
#include <cassert>
namespace cpt { namespace improc {

/**
 * @brief   @copybrief CPT/improc/src_r_o_i_infer.hpp
 */
struct SrcROIInfer
{
    /// @private 
    void repos_gl ( std::vector<uint32_t>& gl, uint32_t start_pos, uint32_t line_num )
    {
        auto base = gl.at(start_pos);
        for ( uint16_t i = start_pos; i < start_pos + line_num; i ++ )
        {
            gl.at( i - start_pos) =  ( gl.at( i ) - base );
        }
        gl.resize( line_num );
    }
    /// @private 
    void resize_tiles( 
          std::vector<cv::Rect>&        tiles
        , const cv::Rect&               roi
        , const std::vector<uint32_t>&  gl_x
        , const std::vector<uint32_t>&  gl_y
    )
    {
        std::vector<cv::Rect> new_tiles;
        new_tiles.resize( roi.width * roi.height );
        auto&& rmv = cpt::view::make_row_major_view( tiles, gl_x.size() - 1, gl_y.size() - 1 );
        auto&& new_rmv = cpt::view::make_row_major_view( new_tiles, roi.width, roi.height );
        auto x_offset = gl_x.at(roi.x);
        auto y_offset = gl_y.at(roi.y);
        for ( int i = roi.y; i < roi.y + roi.height; i ++ )
        {
            for ( int j = roi.x; j < roi.x + roi.width; j ++ )
            {
                auto& tile = rmv( i, j );
                tile.x -= x_offset;
                tile.y -= y_offset;
                new_rmv(i - roi.y, j - roi.x) = tile;
            }
        }
        tiles.swap( new_tiles );
        assert( tiles.size() == tiles.capacity() );
    }
  public:
    /**
     *  @brief      @copybrief CPT/improc/src_r_o_i_infer.hpp 
     *  @param  roi     The ROI information from roi step
     *  @param  src     The raw image
     *  @param  gl_x    x direction grid line
     *  @param  gl_y    y direction grid line
     *  @param  tiles   The tile set 
     *  @param  enable  Run this step or not
     *  @param  v_final Show grid result
     *  @param  verbose Set to false if no image shown are needed ( will override other "v_" prefix variable ), else set to true.
     */
    auto operator()(
          const cv::Rect& roi
        , const cv::Mat& src
        , std::vector<uint32_t>& gl_x
        , std::vector<uint32_t>& gl_y
        , std::vector<cv::Rect>& tiles
        , const bool& enable
        , const int16_t& v_final
        , const bool& verbose
    )
    {
        if ( enable )
        {
            cv::Rect src_roi(
                  cv::Point( gl_x.at(roi.x), gl_y.at(roi.y) )
                , cv::Point( 
                      gl_x.at(roi.x + roi.width )
                    , gl_y.at(roi.y + roi.height )
                )
            );
            auto res = src( src_roi );
            resize_tiles( tiles, roi, gl_x, gl_y );
            repos_gl( gl_x, roi.x, roi.width + 1 );
            repos_gl( gl_y, roi.y, roi.height + 1 );
            cv_imshow(
                v_final
                , [&]{
                    auto tmp = res.clone();
                    tmp = trim_outlier( (cv::Mat_<uint16_t>&)tmp, 0, 0.02 );
                    cv::normalize( 
                          tmp, tmp, 0, 65535
                        , cv::NORM_MINMAX, CV_16U
                    );
                    return tmp;
                }
                , verbose
            );
            return res;
        }
        return src;
    }
};

}}
