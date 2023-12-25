/**
 *  @file    CPT/improc/segmentation.hpp
 *  @author  Alex Lee
 *  @brief   Adjustment the pixel padding and margin size of grid.
 */
#pragma once
#include <CPT/improc/util.hpp>
namespace cpt { namespace improc
{

/**
 *  @brief   Adjustment the pixel padding and margin size of grid.
 *  @details The detail information can see here @ref improc_segmentation
 */
struct Segmentation
{
    /**
     *  @brief  Adjustment the pixel padding and margin size of grid.
     *  @param  src             The input image
     *  @param  cell_margin     The cell margin information 
     *  @param  tiles           The grid cells
     *  @param  v_final         Show the segment result
     *  @param  verbose         Set to false if no image show process are need ( will override other "v_" prefix variable ), else set to true.
     */
    auto operator()(
          const cv::Mat& src
        , const std::vector<int32_t>& cell_margin
        , std::vector<cv::Rect>& tiles
        , const int16_t& v_final
        , const bool& verbose = true
    )
    {
        const auto& m = cell_margin;
        const auto width  = m[2] + m[3];
        const auto height = m[0] + m[1];

        for (auto& tile: tiles)
        {
            if (tile.width > width)
            {
                tile.x += m[2];
                tile.width -= m[2] + m[3];
            }
            if (tile.height > height)
            {
                tile.y += m[0];
                tile.height -= m[0] + m[1];
            }
        }

        // draw gridding result
        cv_imshow(
            v_final
          , [&src, &tiles]
            {
                auto tmp = src.clone();
                const auto color = cv::Scalar(cmax(tmp) * 0.5);
                for (auto tile: tiles)
                {
                    tile.width  += 1;
                    tile.height += 1;
                    cv::rectangle(tmp, tile, color);
                }
                return tmp;
            }
          , verbose
        ); 
    }
};

}}
