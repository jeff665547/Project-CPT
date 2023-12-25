#pragma once
#include <utility>
#include <algorithm>
#include <boost/range/distance.hpp>
#include <vector>

namespace cpt {
namespace algorithm {
#define RM_SAM(binexp1, binexp2) \
    while (first1 != last1) { \
        if (first2 == last2) return std::copy(first1, last1, d_first); \
        if (binexp1) { \
            *d_first++ = *first1++; \
        } else { \
            if (!binexp2 ) { \
                ++first1; \
            } \
            if ( binexp2 ) { \
                ++ first2; \
            } \
        } \
    } \
    return d_first;
template<class InputIt1, class InputIt2, class OutputIt>
OutputIt rm_same (
    InputIt1 first1, InputIt1 last1,
    InputIt2 first2, InputIt2 last2,
    OutputIt d_first
)
{
    RM_SAM((*first1 < *first2), (*first2 < *first1))
}
template<class InputIt1, class InputIt2, class OutputIt, class PRED>
OutputIt rm_same (
      InputIt1 first1, InputIt1 last1
    , InputIt2 first2, InputIt2 last2
    , OutputIt d_first
    , PRED&& pred
)
{
    RM_SAM(pred(*first1, *first2), pred(*first2, *first1))
}

struct RmSam{};

template<class R1, class R2>
auto operator|( R1&& r1, std::tuple<RmSam, R2> rt2 )
{
    std::vector<typename std::decay_t<R1>::value_type> res;
    auto&& r2( std::get<1>(rt2) );
    return rm_same(
        r1.begin()
        , r1.end()
        , r2.begin()
        , r2.end()
        , std::inserter(res, res.begin())
    );
}
}}
