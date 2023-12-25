#pragma once
#include <CPT/range/core.hpp>
#include <CPT/range/segment/detail.hpp>
namespace cpt {
namespace range {

struct Segmented
{
    const std::size_t segsize;

    template<class RNG>
    decltype(auto) operator()(RNG&& rng)
    {
        return std::forward<RNG>(rng) 
            | glue(segment::SegmentedProto{segsize});
    }
};

auto segmented(std::size_t segsize)
{
    return Segmented { std::move ( segsize ) };
}

template<class RNG>
auto operator|( RNG&& rng, Segmented seg )
{
    return seg( std::forward<RNG>(rng) );
}

}}
