#pragma once
#include <utility>
#include <algorithm>
#include <boost/range/distance.hpp>
#include <vector>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <Nucleona/tuple.hpp>

namespace cpt {
namespace algorithm {

template<class R1, class R2>
struct SetDiff
{
    R1 r1;
    R2 r2;
    SetDiff(R1&& r1, R2&& r2)
    : r1(std::forward<R1>(r1))
    , r2(std::forward<R2>(r2))
    {}
};
template<class R1, class R2>
auto make_set_diff( R1&& r1, R2&& r2 )
{
    return SetDiff<R1, R2>(
          std::forward<R1> ( r1 )
        , std::forward<R2> ( r2 )
    );
}
#define SET_DIFF_OR_OP \
    orng.resize( boost::distance( sd.r1 ) ); \
    auto itr = std::set_difference( \
          sd.r1.begin() \
        , sd.r1.end() \
        , sd.r2.begin() \
        , sd.r2.end() \
        , orng.begin() \
    ); \
    orng.resize( itr - orng.begin() ); \
    return orng;

template<class O_RNG, class R1, class R2>
auto operator|( O_RNG&& orng, SetDiff<R1, R2>& sd )
{
    SET_DIFF_OR_OP
}
template<class O_RNG, class R1, class R2>
auto operator|( O_RNG&& orng, SetDiff<R1, R2>&& sd )
{
    SET_DIFF_OR_OP
}
#undef SET_DIFF_OR_OP
template<class R1, class R2>
auto diff_range( R1&& r1, R2&& r2 )
{
    return std::vector<typename R1::value_type>()
        | make_set_diff( std::forward<R1>(r1), std::forward<R2>(r2) )
    ;
}



struct SetDiffTag
{};

template<class R2>
auto set_diff( R2&& r2 )
{
    return nucleona::make_tuple(
          SetDiffTag()
        , std::forward<R2>(r2)
    );
}

template<class R1, class R2>
auto operator|( R1&& r1, std::tuple<SetDiffTag, R2> rt2)
{
    std::vector<typename std::decay_t<R1>::value_type> res;
    auto&& r2 = std::get<1>(rt2);
    for ( auto&& i : r1 )
    {
        bool any_hit = false;
        for ( auto&& j : r2 )
        {
            if ( i == j ) 
            {
                any_hit = true;
                break;
            }
        }
        if ( !any_hit )
        {
            res.emplace_back ( i );
        }
    }
    // boost::set_difference( 
    //       std::forward<R1>(r1)
    //     , std::forward<R2>(std::get<1>(rt2))
    //     , std::inserter(res, res.begin())
    // ); 
    return res;
}
}}
