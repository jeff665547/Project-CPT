#pragma once
#include <boost/range/irange.hpp>
namespace cpt {
namespace utility {
template<class I>
decltype(auto) irange_0( I&& i)
{
    return boost::irange((std::decay_t<I>)0, std::forward<I>(i) );
}
template<class... T>
decltype(auto) irange( T&&... o)
{
    return boost::irange( std::forward<T>(o)... );
}
// template<class... T>
// decltype(auto) ssirange( T&&... o)
// {
// }

}}
