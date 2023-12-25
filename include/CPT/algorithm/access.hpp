#pragma once
#include <utility>
#include <boost/range/adaptors.hpp>
#include <Nucleona/tuple.hpp>
#include <boost_addon/range_vector.hpp>
#include <boost_addon/range_glue.hpp>
#include <CPT/utility/mutable.hpp>
#include <CPT/algorithm/lazy_foreach.hpp>
namespace cpt {
namespace algorithm {
namespace ba = boost::adaptors;
namespace cu = cpt::utility;
struct IdxAccessTag{};
template<class RNG>
struct IdxValueAdapter : public cu::MutableStorage<RNG>
{
    using Base = cu::MutableStorage<RNG>;
    IdxValueAdapter(RNG&& rng)
    : Base{ std::forward<RNG>( rng ) }
    {
    }

    template<class I>
    decltype(auto) operator()(I i) const
    {
        return Base::storage[i];
    }
};
template<class RNG>
auto make_idx_value_adapter(RNG&& rng)
{
    return IdxValueAdapter<RNG>(std::forward<RNG>(rng));
}
template<class RNG, class IdxRng>
auto operator|(
      RNG&& rng
    , std::tuple<IdxAccessTag, IdxRng> tup
)
{
    return
        std::forward<IdxRng>(std::get<1>(tup))
        | cpt::algorithm::lazy_foreach(
            make_idx_value_adapter(std::forward<RNG>(rng))
        )
        
    ;
}

template<class IDXS>
auto idx_access( IDXS&& idxs )
{
    return nucleona::make_tuple(
          IdxAccessTag()
        , ::make_vector(std::forward<IDXS>(idxs))
    );
}
template<class T>
auto idx_access( const std::initializer_list<T>& idxs )
{
    return idx_access(::make_vector(idxs));
}
}}
