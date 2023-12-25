#pragma once
#include <CPT/improc/util.hpp>
#include <CPT/utility/assert.hpp>
namespace cpt { namespace improc
{

struct MiddleSegmentation
{
    auto operator()(
          const cv::Mat& src
        , const int16_t& width
        , const int16_t& height
        , std::vector<cv::Rect>& tiles
        , const std::string& img_path
        , const int16_t& v_final
        , const bool& verbose = true
    )
    {

        for (auto& tile: tiles)
        {
            auto x_off = ( tile.width  - width  ) / 2;
            auto y_off = ( tile.height - height ) / 2;
            cpt::utility::throw_if( x_off < 0 );
            cpt::utility::throw_if( y_off < 0 );

            tile.x += x_off;
            tile.y += y_off;
            tile.width = width;
            tile.height = height;
            
        }

        // draw gridding result
        cv_imshow(
              boost::filesystem::path(img_path)
                .stem().string() + "_middle_segmentation.tif"
            , [&src, tiles] () mutable
            {
                cv::Mat_<uint16_t> tmp = src.clone();
                trim_outlier( tmp, 0, 0.5 );
                cv::normalize( tmp, tmp, 0, 65535, cv::NORM_MINMAX );
                const auto color = cv::Scalar(cmax(tmp) * 0.5);
                for (auto tile: tiles)
                {
                    cv::rectangle(tmp, tile, color);
                }
                return tmp;
            }
            , verbose
        ); 
    }
};

}}
