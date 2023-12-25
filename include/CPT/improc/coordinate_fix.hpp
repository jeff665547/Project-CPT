/**
 * @file    CPT/improc/coordinate_fix.hpp
 * @author  Chia-Hua Chang
 * @brief   Fix the coordinate system of image to fit the probe spec.
 */
#pragma once
#include <CPT/improc/util.hpp>
#include <CPT/improc/chip_mark_layout.hpp>
#include <CPT/improc/coordinate_system_normalization.hpp>
#include <CPT/view.hpp>
#include <CPT/improc/r_o_i_qc.hpp>
namespace cpt { namespace improc {


/**
 * @brief   The region of Interest detection algorithm implementation.
 * @details The detail information can see here @ref improc_coordinate_fix
 */
constexpr struct CoordinateFix
{
    template<class T>
    static void grid_normalize( 
          cv::Mat_<T>& m
        , const cv::Rect& roi
        , const std::string& origin_position
        , char x_axis_direction
    )
    {
        CoordinateSystemNormalization csn;
        csn.operator()( m, origin_position, x_axis_direction );
    }
    template<class T>
    static void grid_normalize_vec( 
          std::vector<T>& vec
        , std::size_t col_nums
        , const cv::Rect& roi
        , const std::string& origin_position
        , char x_axis_direction
    )
    {
        CoordinateSystemNormalization csn;
        csn.operator()( vec, roi.width, origin_position, x_axis_direction );
    }
    /**
     * @brief         @copybrief CPT/improc/coordinate_fix.hpp
     * @details       The coordinate system of image and probe spec may not matched, 
     *                and this step read the user specified image coordinate to rotate or flip the image 
     *                to match the probe spec, which means the position (0,0) of the probe grid should be 
     *                the probe which's id is 0.
     *
     * @param   mean                The mean value of intensities grid.
     * @param   stddev              The standard deviation of the intensities grid.
     * @param   pixels              The pixel of raw image
     * @param   cv_mat              The matrix of cv relate to every probe. ( can be empty matrix )
     * @param   detail_raw_values   The detail pixel values in every probe. ( can be empty vector )
     * @param   roi                 The ROI result of ROI process.
     * @param   origin_position     The origion position of the image, 
     *                              can be "LT" (left top) or "LB" (left bottom). 
     *                              This is used to fix the coordinate system.
     * @param   x_axis_direction    The origion position of the image, can be '_' (horizontal) 
     *                              or '|' (vertical). This is used to fix the coordinate system.
     */
    auto operator() (
          cv::Mat_<float>& mean
        , cv::Mat_<float>& stddev
        , cv::Mat_<uint16_t>& pixels
        , cv::Mat_<float>& cv_mat
        , std::vector<cv::Mat_<int>>& detail_raw_values
        , cv::Rect& roi
        , const std::string& origin_position
        , const char& x_axis_direction
    ) const
    {
        if ( detail_raw_values.size() > 0 )
        {
            grid_normalize_vec(
                  detail_raw_values
                , mean.cols
                , roi
                , origin_position
                , x_axis_direction
            );
        }
        grid_normalize( 
              mean, roi
            , origin_position
            , x_axis_direction 
        );
        grid_normalize( 
              stddev, roi
            , origin_position
            , x_axis_direction 
        );
        grid_normalize( 
              pixels, roi
            , origin_position
            , x_axis_direction 
        );
        if ( cv_mat.cols > 0 && cv_mat.rows > 0 )
        {
            grid_normalize(
                  cv_mat, roi
                , origin_position
                , x_axis_direction
            );
        }

        // grid_normalize_tile(
        //       tiles, mean.cols
        //     , gl_x
        //     , gl_y
        //     , roi
        //     , origin_position
        //     , x_axis_direction
        // );
                  
    } 
} coordinate_fix;


}}
