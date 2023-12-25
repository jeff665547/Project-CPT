#pragma once
#include <map>
namespace cpt {
namespace algorithm {

template<class RNG>
auto acc_count ( RNG&& rng )
{
    std::map<typename std::decay_t<RNG>::value_type, std::size_t> res;
    for( auto&& v : rng ) { res[v]++; }
    return res;
}

}}
