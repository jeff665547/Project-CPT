#pragma once
#include <algorithm>
#include <utility>
#include <Nucleona/tuple.hpp>
namespace cpt {
namespace algorithm {

class SortTag{};

template<class RNG>
auto operator|( RNG&& rng, SortTag o )
{
    std::sort(rng.begin(), rng.end());
    return std::forward<RNG>(rng);
}
template<class RNG, class COMP>
auto operator|( RNG&& rng, std::tuple<SortTag, COMP> o )
{
    std::sort(
          rng.begin()
        , rng.end()
        , std::forward<COMP>(std::get<1>(o))
    );
    return std::forward<RNG>(rng);
}
auto sort()
{
    return SortTag();
}
template<class COMP>
auto sort(COMP&& comp)
{
    return nucleona::make_tuple(SortTag(), std::forward<COMP>(comp));
}

}}
