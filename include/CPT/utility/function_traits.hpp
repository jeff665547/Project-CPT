#pragma once
#include <boost/type_traits/function_traits.hpp>
#include <iostream>
namespace cpt {
namespace utility {
template<class F>
struct FunctionTraits : public FunctionTraits<decltype(&std::decay_t<F>::operator())>
{};
template<class R, class T, class...ARGS>
struct FunctionTraits<R(T::*)(ARGS...) const> : public boost::function_traits<R(ARGS...)>
{
};
template<class R, class T, class...ARGS>
struct FunctionTraits<R(T::*)(ARGS...)> : public boost::function_traits<R(ARGS...)>
{
};
template<class R, class...ARGS>
struct FunctionTraits<R(*)(ARGS...)> : public boost::function_traits<R(ARGS...)>
{
};
template<class R, class...ARGS>
struct FunctionTraits<R(&)(ARGS...)> : public boost::function_traits<R(ARGS...)>
{
};
}}
