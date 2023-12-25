#pragma once
#include <boost/type_traits.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator.hpp>
#include <boost/optional.hpp>
#include <functional>
#include <boost/range/iterator_range.hpp>
#include <Nucleona/language.hpp>
#include <CPT/utility/mutable.hpp>
#include <boost_addon/function_range/core.hpp>
#include <CPT/range/core.hpp>
#include <CPT/utility/irange.hpp>
#include <CPT/range/segment/iterator.hpp>
namespace cpt {
namespace range {
namespace segment {
RANGE_NAMESPACE_SHORTCUT

struct SegmentedProto
{
    const std::size_t segsize;
    template<class RNG>
    auto operator() ( RNG& rng ) const
    {
        return boost::make_iterator_range(
              make_segment_iter     ( rng, segsize )
            , make_segment_end_iter ( rng, segsize )
        );
    }
};
template<class RNG>
auto operator|( RNG& rng, SegmentedProto segment_proto )
{
    return segment_proto( rng );
}



}}}
