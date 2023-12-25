#pragma once
#include <Nucleona/tuple.hpp>
#include <algorithm>
#include <numeric>
namespace cpt {
namespace algorithm {

struct Accumulate{};

template<class... T>
using AccumulateF = std::tuple<
      Accumulate
    , T...
>;

template<class INI, class BF>
auto accumulate(INI&& ini, BF&& bf)
{
    return nucleona::make_tuple(
          Accumulate()
        , std::forward<INI>(ini)
        , std::forward<BF>(bf)
    );
}
template<class INI>
auto accumulate(INI&& ini)
{
    return nucleona::make_tuple(
          Accumulate()
        , std::forward<INI>(ini)
    );
}

template<class RNG, class INI>
auto operator|( RNG&& rng, AccumulateF<INI> a )
{
    return std::accumulate(
          rng.begin()
        , rng.end()
        , std::get<1>(a)
    );
}
template<class RNG, class INI, class BF>
auto operator|( RNG&& rng, AccumulateF<INI, BF> a )
{
    return std::accumulate(
          rng.begin()
        , rng.end()
        , std::get<1>(a)
        , std::get<2>(a)
    );
}

}}
