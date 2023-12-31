/**
 * @file    CPT/improc/rotation_estimation.hpp
 * @author  Alex Lee, Chia-Hua Chang
 * @brief   Estimate the rotation angle of image need to be corrected.
 */
#pragma once
#include <cmath>
#include <random>
#include <CPT/improc/util.hpp>
#include <Nucleona/parallel/thread_pool.hpp>
#include <CPT/improc/hough_transform.hpp>
#include <atomic>
namespace cpt { namespace improc
{
/**
 * @brief  Estimate the rotation angle of image need to be corrected.
 * @tparam FLOAT The float point type used
 * @details The detail information can see here @ref improc_image_rotation
 */
template<class FLOAT>
class RotationEstimation
{
  protected:
    struct UnitVector
    {
        FLOAT theta;
        FLOAT cos_theta;
        FLOAT sin_theta;

        UnitVector(FLOAT theta)
          : theta(theta)
          , cos_theta(std::cos(theta * CV_PI / 180.0))
          , sin_theta(std::sin(theta * CV_PI / 180.0))
        {}

        FLOAT rho(const int32_t x, const int32_t y) const
        {
            return x * cos_theta + y * sin_theta;
        }
    };

    static FLOAT rho(const int32_t x, const int32_t y, const FLOAT theta)
    {
        return x * std::cos(theta * CV_PI / 180.0)
             + y * std::sin(theta * CV_PI / 180.0);
    }
    static auto sample( 
          cv::Mat& m
        , uint32_t sample_width
        , uint32_t sample_height 
    )
    {
        cv::Point start( (m.cols - sample_width) / 2, (m.rows - sample_height ) / 2  );
        cv::Point end ( start.x + sample_width, start.y + sample_height );
        return m( cv::Rect( start, end ) );
    }
    static auto sample( 
          const cv::Mat& m
    )
    {
        return m;
    }
    template<class T>
    static decltype(auto) get_value( T&& v ) { return v; }
    template<class T>
    static decltype(auto) get_value( const std::atomic<T>& v ) { return v.load(std::memory_order_relaxed); }
    template<class T>
    static auto min_entropy( cv::Mat_<T>& m )
    {
        cv::Point res;
        FLOAT res_val(std::numeric_limits<FLOAT>::max());
        for ( int i = 0; i < m.rows; i ++ )
        {
            FLOAT sum(0);
            for ( int j = 0; j < m.cols; j ++ )
            {
                sum += get_value(m( i, j ));
            }
            if ( std::abs(sum - 0.0) <= std::numeric_limits<FLOAT>::epsilon() )
            { continue; }
            else
            {
                FLOAT entropy(0);
                for ( int j = 0; j < m.cols; j ++ )
                {
                    auto& t = get_value(m ( i, j ));
                    auto p = t / sum ;
                    if ( std::abs(p - 0.0) > std::numeric_limits<FLOAT>::epsilon() )
                    {
                        entropy -= ( p * std::log( p ) );
                    }
                }
                if ( entropy < res_val ) 
                {
                    res_val = entropy ;
                    res = cv::Point( 0, i );
                }
            }
        }
        cpt::dbg << res_val << std::endl;
        return res;
    }

  public:
    /**
     *  @brief  Estimate the rotation angle of image need to be corrected.
     *  @details Estimate the rotation angle of image
     *  @param in_src            Input image
     *  @param has_grid_img      Flag for grid image provided or not
     *  @param grid_img          Grid image. Pass a empty matrix, if not provided.
     *  @param min_theta         The scan angles' lower bound. Given a angle range to be scan, as small as fast
     *  @param max_theta         The scan angles' upper bound. Given a angle range to be scanned, as small as faster.
     *  @param steps             The scan angles' step interval as big as faster, but accuracy will lower.
     *  @param v_edges           Show the edge image.
     *  @param v_hough           Show the histogram
     *  @param verbose           Set to false if no image show process are need ( will override other "v_" prefix variable ), else set to true.
     */
    auto operator()( 
          cv::Mat&                  in_src
        , const bool&               has_grid_img
        , cv::Mat&                  grid_img
        , const FLOAT&              min_theta
        , const FLOAT&              max_theta 
        , const FLOAT&              steps
        , const int16_t&            v_edges
        , const int16_t&            v_hough
        , bool                      verbose = true
    )
    {
        cv::Mat src = sample ( has_grid_img ? grid_img : in_src.clone() );
#ifdef ADDITIONAL_SHOW
        cv_imshow(
            0
            , [m = (cv::Mat_<float>)src.clone()] () mutable 
            { 
                std::cout << "sample" << std::endl;
                m = trim_outlier( m, 0, 0.02 );
                cv::normalize( m, m, 0, 1, cv::NORM_MINMAX, CV_32F );
                return m; 
            }
            , verbose
        );
#endif
        HoughTransform<FLOAT> hough_transform( min_theta, max_theta, steps );
        
        if ( !has_grid_img )
        {
            cv::Mat padded;
            int m = cv::getOptimalDFTSize(src.rows);
            int n = cv::getOptimalDFTSize(src.cols);
            cv::blur(src, src, cv::Size(3, 3));
            cv::copyMakeBorder(src, padded, 0, m - src.rows, 0, n - src.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

            cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat_<float>::zeros(padded.size()) };
            cv::Mat merged;
            cv::merge(planes, 2, merged);
            cv::dft(merged, merged, cv::DFT_SCALE | cv::DFT_COMPLEX_OUTPUT);
            cv::split(merged, planes); 

            cv::Mat_<float> magnitude, phase;
            cv::magnitude(planes[0], planes[1], magnitude);
            cv::phase(planes[0], planes[1], phase);

            cv::Mat_<float> north = (
                decltype(north)(3, 3)
                <<  1,  2,  1
                 ,  0,  1,  0
                 , -1, -2, -1
            );
            cv::filter2D(magnitude, magnitude, -1, north);


            cv::polarToCart(magnitude, phase, planes[0], planes[1]);
            cv::merge(planes, 2, merged);
            cv::dft(merged, merged, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT);
            cv::split(merged, planes);


            cv::filter2D(planes[0], src, -1, north);
            src = cv::max(src, 0);

            north(1, 1) = 0;
            cv::Mat_<float> south = (
                decltype(south)(3, 3)
                << -1, -2, -1
                 ,  0,  0,  0
                 ,  1,  2,  1
            );
            for (auto i = 0; i != 3; ++i)
            {
                cv::filter2D(src, src, -1, north);
                cv::filter2D(src, src, -1, south);
            }
            src = cv::max(src, 0);
            cv::normalize(src, src, 0.0, 255.0, cv::NORM_MINMAX, CV_8U);

            cv::Mat unused;
            auto thres = cv::threshold(src, unused, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
            cv::threshold( src, src, thres * 0.8, 255.0, cv::THRESH_BINARY );
        }
        else
        {
            cv::normalize(src, src, 0.0, 255.0, cv::NORM_MINMAX, CV_8U);

            cv::Mat unused;
            auto thres = cv::threshold(src, unused, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
            cv::Canny(src, src, thres * 0.95, cv::saturate_cast<uint8_t>(thres * 1.0));
        }

        cv_imshow(
            v_edges
          , [&src]{ return src; }
          , verbose
        );
        auto hist = hough_transform( src );
        cv::Point loc = min_entropy( hist );
        // double val;
        // cv::minMaxLoc(hist, nullptr, &val, nullptr, &loc);
        auto theta = hough_transform.unitvecs()[loc.y].theta - 90.0;

        cpt::msg << "theta = " << theta << '\n';

        cv_imshow(
            v_hough
          , [&hist, &loc](void)
            {
                cv::normalize(hist, hist, 0.0, 1.0, cv::NORM_MINMAX, hist.depth());
                cv::line(hist, cv::Point(0, loc.y), cv::Point(hist.cols-1, loc.y), cv::Scalar(1.0));
                return hist;
            }
          , verbose
        );
        return theta;
    }
};


}}
