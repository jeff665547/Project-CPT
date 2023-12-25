#pragma once
#include <vector>
#include <CPT/improc/util.hpp>
#include <CPT/algorithm/paralleled.hpp>
#include <CCD/utility/tune_scope.hpp>
#include <Nucleona/parallel/thread_pool.hpp>

namespace cpt { namespace improc
{
template<class FLOAT>
struct HoughTransform
{
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
    HoughTransform(FLOAT tmin, FLOAT tmax, FLOAT steps )
    : tmin_ ( tmin  )
    , tmax_ ( tmax  )
    , tstep_( (tmax - tmin )/steps)
    {
        for (auto theta = tmin; theta < tmax; theta += tstep_)
            unitvecs_.emplace_back(theta);
    }

    const auto& unitvecs()
    {
        return unitvecs_;
    }

    auto operator()( cv::Mat img )
    {
        const auto cols = img.cols;
        const auto rows = img.rows;
        const auto tmid = std::atan2(rows, cols) * 180.0 / CV_PI;
        const auto rmin = std::floor(
            (tmax_ > 90.0)? rho(cols, 0, tmax_): 0.0
        );
        const FLOAT rmax = std::ceil(
            (tmax_ < tmid)? rho(cols, rows, tmax_)
          : (tmin_ < tmid)? std::sqrt(rows * rows + cols * cols)
          : (tmin_ < 90.0)? rho(cols, rows, tmin_)
          :                rho(0   , rows, tmin_)
        );
        cv::Mat_<std::atomic<FLOAT>> hist = cv::Mat_<std::atomic<FLOAT>>::zeros(unitvecs_.size(), rmax - rmin + 1);
        const auto thread_num = 4;
        {
        ::LightTuneScope<LightTuneScopeNoPool> timer("hough_transform");
        auto thread_pool(nucleona::parallel::make_thread_pool(thread_num));
        for ( auto r_pack : cpt::utility::irange_0( img.rows ) 
                | cpt::range::segmented( img.rows / thread_num ) )
        {
            thread_pool.job_post([ rp = std::move(r_pack), &img, this, &hist, &rmin ]() mutable
            {
                for (auto r : rp)
                    for (auto c = 0; c != img.cols; ++c)
                        if (img.template at<uint8_t>(r, c) > 127) // and (std::rand() & 0x4) == 0)
                            for (auto& u: unitvecs_ )
                            {
                                auto hi = std::round((u.theta     - tmin_) / tstep_);
                                auto hj = std::round((u.rho(c, r) - rmin)          );
                                auto v = hist( hi, hj ).load( std::memory_order_acquire );
                                hist(hi, hj).compare_exchange_weak( v, v + 1.0
                                    , std::memory_order_release
                                    , std::memory_order_relaxed 
                                );
                            }
            });
        }
        thread_pool.flush();
        }
        
        return hist;
    
    }
private:
    std::vector<UnitVector> unitvecs_;
    FLOAT tmin_, tmax_, tstep_;
};

}}
