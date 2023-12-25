#pragma once
#include <CPT/improc/util.hpp>
#include <CPT/improc/filters/mat.hpp>
#include <CPT/utility/language.hpp>
namespace cpt{ namespace improc{ namespace filter{

constexpr struct Exculde
{
    template<class M>
    auto operator()( M&& mat, const std::vector<cv::Rect>& mask ) const
    {
        return Mat<M>( FWD(mat), mask );
    }
} exclude;

}}}
