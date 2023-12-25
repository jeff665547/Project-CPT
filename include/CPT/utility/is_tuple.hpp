#pragma once
#include <type_traits>
#include <tuple>
namespace cpt { namespace utility {

template<class T>       struct is_tuple_type : public std::false_type {};
template<class... T>    struct is_tuple_type<std::tuple<T...>> : public std::true_type {};

}}
