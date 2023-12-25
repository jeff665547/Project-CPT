#pragma once
#include <utility>
namespace cpt {
namespace algorithm {

struct Grouped
{
    std::size_t n;
};

template<class RNG>
auto operator|( RNG&& rng, Grouped grouped )
{
    // return 
} 

template<class INT>
auto grouped(INT&& i)
{
    return Grouped(i);
}

}}
