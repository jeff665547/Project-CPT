#pragma once

#include <utility>
#include <boost/range/distance.hpp>

namespace cpt {
namespace algorithm {

struct EvalRangeDistance
{
};

constexpr EvalRangeDistance eval_distanced = EvalRangeDistance();

template<class RNG>
auto operator|(RNG&& rng, EvalRangeDistance rd)
{
    std::size_t n(0);
    for( auto&& i : rng )
    {
        n++;
    }
    return n;
}
struct RangeDistance
{
};

constexpr RangeDistance distanced = RangeDistance();

template<class RNG>
auto operator|(RNG&& rng, RangeDistance rd)
{
    return boost::distance(rng);
}

}}
