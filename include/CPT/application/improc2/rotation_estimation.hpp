#pragma once
#include "utils.hpp"
#include <cmath>
#include <random>
#include <CPT/improc/rotation_calibration.hpp>
#include <CPT/application/intensities_dump/debug_args.hpp>

namespace cpt {
namespace application {
namespace improc2 {

// class RotationCalibration
// {
//     using This = RotationCalibration;
// 
//   public:
//     struct Parameters
//     {
//         int32_t v_final;
// 
//         template <class BUFFER, class JSON>
//         static void run(BUFFER&& buf, JSON&& json)
//         {
//             auto&& opts = json.get_child("rotation");
//             buf.This::Parameters::config(opts);
//         }
//         template <class JSON>
//         void config(JSON&& opts)
//         {
//             v_final = opts
//                 .template get_optional<decltype(v_final)>("verbose.final")
//                 .value_or(-1);
// 
//             cpt::msg << "rotation.verbose.final = " << v_final << '\n';
//         }
//     };
// 
//   public:
//     template <class BUFFER>
//     static void run(BUFFER&& buf)
//     {
//         auto& src = buf.src;
//         cv::Point2f center(src.cols >> 1, src.rows >> 1);
//         auto mat = cv::getRotationMatrix2D(center, buf.theta, 1.0);
//         cv::warpAffine(src, src, mat, src.size());
// 
//         buf.show(
//             buf.This::Parameters::v_final
//           , [&src] { return src; }
//         );
//     }
// };
class RotationCalibration
{
    using This = RotationCalibration;

  public:
    struct Parameters
    {
        int32_t v_final;

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("rotation");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
        {
            v_final = opts
                .template get_optional<decltype(v_final)>("verbose.final")
                .value_or(-1);

            cpt::msg << "rotation.verbose.final = " << v_final << '\n';
        }
    };

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        // std::cout << "==========rc=========" << std::endl;
        // std::cout << buf.src(cv::Rect(
        //     0, 0, 50, 50 ) ) << std::endl;
        cpt::improc::RotationCalibration rc;
        rc( buf.src, buf.theta, buf.Parameters::v_final, buf.verbose );
#ifdef SAVE_ROTATE
// this is for debug
        auto fname = boost::filesystem::path(buf.img_path).stem();
        cpt::improc::imwrite( 
              fname.string() + ".after_rotate.tif"
            , buf.src
        );
#endif
    }
};

class RotationEstimation
{
    using This = RotationEstimation;
  protected:
    using FLOAT = float;

  public:
    struct Parameters
    {
        FLOAT min_theta;
        FLOAT max_theta;
        FLOAT steps;
        int32_t v_edges;
        int32_t v_hough;

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("rotation");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
        {
            min_theta = opts
                .template get_optional<FLOAT>("lower")
                .value_or(-5) + 90;
            max_theta = opts
                .template get_optional<FLOAT>("upper")
                .value_or(5) + 90;
            steps = opts
                .template get_optional<FLOAT>("steps")
                .value_or(1000);
            v_edges = opts
                .template get_optional<decltype(v_edges)>("verbose.edges")
                .value_or(-1);
            v_hough = opts
                .template get_optional<decltype(v_hough)>("verbose.hough")
                .value_or(-1);

            cpt::msg << "rotation.verbose.edges = " << v_edges << '\n';
            cpt::msg << "rotation.verbose.hough = " << v_hough << '\n';
        }
    };
 
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

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        auto src = buf.src.clone();
        const auto cols = src.cols;
        const auto rows = src.rows;
        const auto tmin = buf.min_theta;
        const auto tmax = buf.max_theta;
        const auto tmid = std::atan2(rows, cols) * 180.0 / CV_PI;
        const auto rmin = std::floor(
            (tmax > 90.0)? rho(cols, 0, tmax): 0.0
        );
        const FLOAT rmax = std::ceil(
            (tmax < tmid)? rho(cols, rows, tmax)
          : (tmin < tmid)? std::sqrt(rows * rows + cols * cols)
          : (tmin < 90.0)? rho(cols, rows, tmin)
          :                rho(0   , rows, tmin)
        );
        const auto tstep = (tmax - tmin) / buf.steps;

        std::vector<UnitVector> unitvecs;
        for (auto theta = tmin; theta < tmax; theta += tstep)
            unitvecs.emplace_back(theta);

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

            // buf.show(
            //     0
            //   , [m = magnitude.clone()]{ return m; }
            // );
           
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
        }
        cv::normalize(src, src, 0.0, 255.0, cv::NORM_MINMAX, CV_8U);

        // cv::Mat unused;
        // auto thres = cv::threshold(src, unused, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
        // cv::Canny(src, src, thres * 0.95, cv::saturate_cast<uint8_t>(thres * 1.0));
        cv::threshold(src, src, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);

        buf.show(
            buf.This::Parameters::v_edges
          , [&src]{ return src; }
        );

        cv::Mat_<FLOAT> hist = cv::Mat_<FLOAT>::zeros(unitvecs.size(), rmax - rmin + 1);
        for (auto r = 0; r != src.rows; ++r)
            for (auto c = 0; c != src.cols; ++c)
                if (src.template at<uint8_t>(r, c) > 127) // and (std::rand() & 0x4) == 0)
                    for (auto& u: unitvecs)
                        hist(
                            std::round((u.theta     - tmin) / tstep)
                          , std::round((u.rho(c, r) - rmin)        )
                        ) += 1.0;
        double val;
        cv::Point loc;
        cv::minMaxLoc(hist, nullptr, &val, nullptr, &loc);
        buf.theta = unitvecs[loc.y].theta - 90.0;

        cpt::msg << "theta = " << buf.theta << '\n';

        buf.show(
            buf.This::Parameters::v_hough
          , [&hist, &loc](void)
            {
                cv::normalize(hist, hist, 0.0, 1.0, cv::NORM_MINMAX, hist.depth());
                cv::line(hist, cv::Point(0, loc.y), cv::Point(hist.cols-1, loc.y), cv::Scalar(1.0));
                return hist;
            }
        );
    }
};

} // namespace improc2
} // namespace application
} // namespace cpt
