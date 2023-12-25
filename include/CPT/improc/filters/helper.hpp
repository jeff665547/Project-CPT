#pragma once
#include <CPT/improc/util.hpp>
namespace cpt{ namespace improc{ namespace filter{
namespace detail{
template<class M>
static auto rows( M&& m ) -> decltype(m.rows)
{
    return m.rows;
}
template<class M>
static auto rows( M&& m ) -> decltype(m.n_rows())
{
    return m.n_rows();
}
template<class M>
static auto cols( M&& m ) -> decltype(m.cols)
{
    return m.cols;
}
template<class M>
static auto cols( M&& m ) -> decltype(m.n_cols())
{
    return m.n_cols();
}
static auto& unwrap_all( cv::Mat& m )
{
    return m;
}
static auto& unwrap_all( const cv::Mat& m )
{
    return m;
}
template<class T>
static auto& unwrap_all( cv::Mat_<T>& m )
{
    return m;
}
template<class T>
static auto& unwrap_all( const cv::Mat_<T>& m )
{
    return m;
}
template<class T>
static auto& unwrap_all( std::vector<T>& v )
{
    return v;
}
template<class T>
static auto& unwrap_all( const std::vector<T>& v )
{
    return v;
}
template<class M>
static auto unwrap_all( M& m ) -> decltype(unwrap_all(m.data_));

template<class M>
static auto unwrap_all( M& m ) -> decltype(unwrap_all(m.core));


template<class M>
static auto unwrap_all( M& m ) -> decltype(unwrap_all(m.data_))
{
    return unwrap_all(m.data_);
}
template<class M>
static auto unwrap_all( M& m ) -> decltype(unwrap_all(m.core))
{
    return unwrap_all(m.core);
}



}
}}}
