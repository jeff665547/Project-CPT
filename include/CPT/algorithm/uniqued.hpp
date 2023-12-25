#pragma once
#include <utility>
#include <algorithm>
#include <functional>
#include <CPT/utility/typecheck.hpp>
namespace cpt {
namespace algorithm {

template<class ForwardIt, class BinaryPredicate, class DiffEvent, class SameEvant>
ForwardIt unique(
    ForwardIt first
    , ForwardIt last
    , BinaryPredicate p      = std::equal_to<>()
    , DiffEvent&& diff_event = [](auto&& r, auto&& f) { r = std::move(f); }
    , SameEvant&& same_event = [](auto&& r, auto&& f) {}
)
{
    if (first == last)
        return last;

    ForwardIt result = first;
    while (++first != last) {
        if (!p(*result, *first) && ++result != first) {
            // *result = std::move(*first);
            diff_event(*result, *first);
        } else {
            same_event(*result, *first);
        }
    }
    return ++result;
}
template<class BinaryPredicate, class DiffEvent, class SameEvant>
struct Uniqued
{
    BinaryPredicate p           ;
    DiffEvent       diff_event  ;
    SameEvant       same_event  ;
};
CREATE_TYPECHECKER(Uniqued);
template<class BinaryPredicate, class DiffEvent, class SameEvant>
auto uniqued(
      BinaryPredicate p      = std::equal_to<>()
    , DiffEvent&& diff_event = [](auto&& r, auto&& f) { r = std::move(f); }
    , SameEvant&& same_event = [](auto&& r, auto&& f) {}
)
{
    return Uniqued<BinaryPredicate, DiffEvent, SameEvant> { 
          std::forward<BinaryPredicate>(p)
        , std::forward<DiffEvent>(diff_event)
        , std::forward<SameEvant>(same_event)
    };
}

template<class RNG, class TAG, FTP_TYPE_CHECK(TAG, Uniqued)>
auto operator|( RNG&& rng, TAG&& tag )
{
    // return unique(rng.begi
}

}}
