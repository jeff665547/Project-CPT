/**
 *  @file    CPT/improc/rotation_calibration.hpp
 *  @author  Alex Lee
 *  @brief   Rotate the image by given angle.
 */
#pragma once
#include <cmath>
#include <random>
#include <CPT/improc/util.hpp>

namespace cpt { namespace improc {

/**
 *  @brief   Rotate the image by given angle.
 *  @details Input the angle and image, the function rotate the image in-place.
 *  @details Detail information can see here @ref improc_image_rotation
 */
struct RotationCalibration
{
  public:
    
    /**
     *  @brief Rotate the image by given angle.
     *  @param in_src   The input image.
     *  @param theta    The input rotate angle.
     *  @param v_final  Show the rotate result.
     *  @param verbose  Set to false if no image show process are need ( will override other "v_" prefix variable ), else set to true.
     */
    template<class FLOAT>
    auto operator()( 
          cv::Mat& in_src
        , FLOAT theta
        , int16_t v_final
        , bool verbose = true
    )
    {
        auto& src = in_src;
        cv::Point2f center(src.cols >> 1, src.rows >> 1);
        auto mat = cv::getRotationMatrix2D(center, theta, 1.0);
        cv::warpAffine(src, src, mat, src.size());
        cv_imshow(
            v_final
          , [&src] { return src; }
          , verbose
        );
    }
};
}}
