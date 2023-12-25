#pragma once
#include <opencv2/imgproc/imgproc.hpp>
#include "floating_point_template.hpp"
#include "utils.hpp"

namespace improc {

class RGBMaxFilter
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        std::vector<cv::Mat> dsts;
        cv::split(src, dsts);
        for (int i = 1; i < src.channels(); ++i)
            dsts[0] = cv::max(dsts[0], dsts[i]); 
        src = dsts[0];
        return std::forward<MAT>(src);
    }
};

template <class TYPE, bool normalize = false>
class ConvertTo
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        const int rtype = cv::DataDepth<TYPE>::value;
        double alpha = 1.0;
        if (normalize)
            alpha = cmax(rtype) / cmax(src);
        src.convertTo(src, rtype, alpha);
        return std::forward<MAT>(src);
    }
};

template <int32_t norm_type, class... ARGS>
class Normalization;

template <class DSTTYPE>
class Normalization<cv::NORM_MINMAX, DSTTYPE>
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        const auto depth = cv::DataDepth<DSTTYPE>::value;
        cv::normalize(
            src
          , src
          , 0.0
          , cmax(depth)
          , cv::NORM_MINMAX
          , depth
        );
        return std::forward<MAT>(src);
    }
};

template <int32_t ksize>
class Blur
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        cv::blur(src, src, cv::Size(ksize, ksize));
        return std::forward<MAT>(src);
    }
};

template <int32_t ksize>
class MedianBlur
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        cv::medianBlur(src, src, ksize);
        return std::forward<MAT>(src);
    }
};

template <
    int32_t ksize
  , class SIGMA = REAL_(0.0)
>
class GaussianBlur
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        cv::GaussianBlur(
            src
          , src
          , cv::Size(ksize, ksize)
          , SIGMA::value
        );
        return std::forward<MAT>(src);
    }
};

template <
    int32_t ksize
  , int32_t diameter
  , class sigma_space
  , class sigma_color
>
class BilateralFilter
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        cv::Mat tmp = src.clone();
        cv::bilateralFilter(
            tmp
          , src
          , diameter
          , sigma_color::value
          , sigma_space::value
        );
        return std::forward<MAT>(src);
    }
};

template <
    int32_t ksize
  , class sigma_space
  , class max_sigma_color = double_(20.0)
>
class AdaptiveBilateralFilter
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        cv::Mat tmp = src.clone();
        cv::adaptiveBilateralFilter(
            tmp
          , src
          , cv::Size(ksize, ksize)
          , sigma_space::value
          , max_sigma_color::value
        );
        return std::forward<MAT>(src);
    }
};

template <
    class scale = double_(1.0)
>
class SharpenFilter
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        const auto sc = scale::value;
        static cv::Mat kern = (
            cv::Mat_<double>(3, 3)
            << 0.0,        -sc, 0.0
             , -sc, 1 + 4 * sc, -sc
             , 0.0,        -sc, 0.0
        );
        cv::filter2D(src, src, src.depth(), kern);
        return std::forward<MAT>(src);
    }
};

template <
    int32_t THRESTYPE
  , class thresval = double_(0.0)
>
class Binarization
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        cv::threshold(
            src
          , src
          , thresval::value
          , cmax(src)
          , THRESTYPE
        );
        return std::forward<MAT>(src);
    }
};

template <
    class ratio = double_(0.5)
>
class EdgeDetection
{
  public:
    template <class MAT>
    static MAT run(MAT&& src)
    {
        cv::Mat __attribute__((unused)) unused;
        auto thres = cv::threshold(
            src
          , unused
          , 0.0
          , cmax(src)
          , cv::THRESH_OTSU
          | cv::THRESH_BINARY
        );
        cv::Canny(
            src
          , src
          , thres * ratio::value
          , thres
        );
        return std::forward<MAT>(src);
    }
};

}
