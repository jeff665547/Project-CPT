#pragma once
#include <boost_addon/range_indexed.hpp>
#include <armadillo>
#include <boost/range/adaptors.hpp>
#include <boost/range/irange.hpp>
#include <Nucleona/type_traits/core.hpp>
#include <CPT/algorithm/access.hpp>
namespace cpt {
namespace algorithm {
namespace matrix_range {
template<class T> struct IsArmaMat : public std::false_type  {};
template<class T> struct IsArmaMat<arma::Mat<T>> : public std::true_type {};
template<class T>
constexpr bool is_arma_mat = IsArmaMat<T>::value;


template<class T> struct IsArmaRow : public std::false_type  {};
template<class T> struct IsArmaRow<arma::Row<T>> : public std::true_type {};
template<class T>
constexpr bool is_arma_row = IsArmaRow<T>::value;


template<class T> struct IsArmaCol : public std::false_type  {};
template<class T> struct IsArmaCol<arma::Col<T>> : public std::true_type {};
template<class T>
constexpr bool is_arma_col = IsArmaCol<T>::value;
}}}
