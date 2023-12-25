#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

namespace improc {

const char* depth(const cv::Mat& image)
{
    switch (image.depth())
    {
        case 0: return "CV_8U" ;
        case 1: return "CV_8S" ;
        case 2: return "CV_16U";
        case 3: return "CV_16S";
        case 4: return "CV_32S";
        case 5: return "CV_32F";
        case 6: return "CV_64F";
    }
    return "Undefined";
}
void info(const cv::Mat& image)
{
    std::cerr << "- dims       " << image.dims         << '\n'
              << "- rows       " << image.rows         << " elems" << '\n'
              << "- cols       " << image.cols         << " elems" << '\n'
              << "- total      " << image.total()      << " elems" << '\n'
              << "- channels   " << image.channels()   << '\n'
              << "- elemSize   " << image.elemSize()   << " bytes" << '\n'
              << "- elemSize1  " << image.elemSize1()  << " bytes" << '\n'
              << "- depth      " << depth(image)       << '\n' ;
}

constexpr double cmax(int32_t depth)
{
    switch (depth)
    {
        case 0 : return std::numeric_limits<uint8_t>::max();
        case 1 : return std::numeric_limits<int8_t>::max();
        case 2 : return std::numeric_limits<uint16_t>::max();
        case 3 : return std::numeric_limits<int16_t>::max();
        case 4 : return std::numeric_limits<int32_t>::max();
        default: return 1.0;
    }
}

double cmax(const cv::Mat& image)
{
    return cmax(image.depth());
}

auto figure(
    const std::string& name
  , const int x = 0
  , const int y = 40
  , const int width  = 800
  , const int height = 600
) {
    cv::namedWindow(name, ::CV_WINDOW_NORMAL);
    cv::moveWindow(name, x, y);
    cv::resizeWindow(name, width, height);
    return name;
}

auto imread(const std::string& fname)
{
    return cv::imread(
        fname
      , ::CV_LOAD_IMAGE_ANYCOLOR
      | ::CV_LOAD_IMAGE_ANYDEPTH
    );
}

auto imwrite(const std::string& fname, const cv::Mat src)
{ 
    return cv::imwrite(fname, src);
}

template <class MAT>
MAT imresize(MAT&& src, const double scale)
{
    cv::resize(src, src, cv::Size(), scale, scale, cv::INTER_LINEAR);
    return std::forward<MAT>(src);
}
template <class MAT>
MAT imresize(MAT&& src, const cv::Size& dsize)
{
    cv::resize(src, src, dsize, 0, 0, cv::INTER_LINEAR);
    return std::forward<MAT>(src);
}

template <class MAT>
MAT imrotate(MAT&& src, const double angle, const double scale = 1)
{
    cv::warpAffine(
        src
      , src
      , cv::getRotationMatrix2D(
            cv::Point(src.cols >> 1, src.rows >> 1)
          , angle
          , scale
        )
      , src.size()
      , cv::INTER_LINEAR
      , cv::BORDER_CONSTANT
      , cv::Scalar(0)
    );
    return std::forward<MAT>(src);
}

} // namespace improc
