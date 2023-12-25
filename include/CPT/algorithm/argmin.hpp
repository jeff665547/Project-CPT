#pragma once
#include <map>
#include <boost/range/adaptor/map.hpp>
#include <CPT/utility/typecheck.hpp>
#include <boost/range/irange.hpp>

namespace cpt {
namespace algorithm {
namespace ba = boost::adaptors;

template<class F, class ARG_RNG>
auto argmin(F&& f, ARG_RNG&& arg_rng)
{
    auto itr = arg_rng.begin();
    auto min_arg = *itr;
    itr ++;
    auto min = f(min_arg);
    for( ; itr != arg_rng.end(); itr ++ )
    {
        auto arg = *itr;
        auto tmp = f(arg);
        if( min > tmp ) 
        {
            min = tmp;
            min_arg = arg;
        }
    }
    return min_arg;
}

template<class K, class V>
auto argmin( const std::map<K, V>& m )
{
    return argmin(
          [&m](auto v) { return m.at(v); }
        , m | ba::map_keys
    );
}
template<class R>
auto argmin( const R& m )
{
    auto dis (boost::distance(m));
    return argmin(
          [&m](auto v) { return m.at(v); }
        , boost::irange(decltype(dis)(0), std::move(dis))
    );
}
template<class F>
struct ArgminTag
{
    F&& f;
    template<class ARG_RNG>
    auto operator()( ARG_RNG&& arg_rng )
    {
        return argmin(std::forward<F>(f), std::forward<ARG_RNG>(arg_rng));
    }
};
CREATE_TYPECHECKER(ArgminTag);
template<class F>
auto argmind(F&& f)
{
    return ArgminTag<F>{std::forward<F>(f)};
}
template<class RNG, class TAG, FTP_TYPE_CHECK(TAG, ArgminTag)>
auto operator|(RNG&& rng, TAG&& tag)
{
    return std::forward<TAG>(tag)(std::forward<RNG>(rng));
}
struct IndexMinTag{};
auto indexmind()
{
    return IndexMinTag();
}
template<class RNG>
auto operator|(RNG&& rng, IndexMinTag tag)
{
    return argmin(std::forward<RNG>(rng));
}

}}
