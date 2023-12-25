#pragma once
#include <vector>
#include <cassert>
#include <Nucleona/language.hpp>
#include <string>
#include <CPT/view/fwd.hpp>
namespace cpt{ namespace view{

template<class T> struct MatrixTrait
{
    using I     = typename T::I;
    using J     = typename T::J;
    using Value = typename T::Value;
};

template<class T> struct MatrixTrait<RowMajorView<T>>
{
    using J     = typename RowMajorView<T>::Width;
    using I     = typename RowMajorView<T>::Height;
    using Value = typename RowMajorView<T>::Value;
};

struct MatViewConst
{
    constexpr static const char* const LT = "LT";
    constexpr static const char* const LB = "LB";
};

}}
