#pragma once
#include <boost_addon/range.hpp>
#include <vector>
#include <type_traits>
#include <Nucleona/language.hpp>
namespace cpt {
namespace algorithm {
template<class RNG, class T>
auto state_flatten_range_base(
        RNG&& rng
      , std::vector<T>& res
);
template<bool b>
struct StateFlatten{};
template<>
struct StateFlatten<true>
{
    template<class RNG, class T>
    static auto run(RNG&& rng, std::vector<T>& res)
    {
        for(auto&& v : rng )
        {
            state_flatten_range_base( FWD(v), res );
        }
    }
};
template<>
struct StateFlatten<false>
{
    template<class V, class T>
    static auto run(V&& v, std::vector<T>& res)
    {
        res.emplace_back( std::forward<V>(v) );
    }
};
template<class RNG, class T>
auto state_flatten_range_base(
      RNG&& rng
    , std::vector<T>& res
)
{
    StateFlatten<is_range_v<std::decay_t<RNG>>>::run(
          std::forward<RNG>(rng)
        , res
    );
}
#ifdef ARMA_INCLUDES
template<class E, class T>
auto state_flatten_range_base(
    const arma::Mat<E>& m
    , std::vector<T>& res
)
{
    for ( auto i : m.n_rows )
    {
        for( auto&& e : m.row(i) )
        {
            res.emplace_back(e);
        }
    }
}
#endif
namespace state_flattened_detail{
    
    template<class RNG, bool range_test = is_range_v<std::decay_t<RNG>>> 
    class FlattenedValue{};

    template<class RNG>
    class FlattenedValue<RNG, true> 
    { 
        using DRng      = std::decay_t<RNG>;
        using InnerRng  = typename DRng::value_type;
      public:
        using Type      = typename FlattenedValue<InnerRng, is_range_v<InnerRng>>::Type;
    };
    template<class RNG>
    class FlattenedValue<RNG, false> 
    { 
      public:
        using Type = RNG;
    };
}
struct StateFlattenTag
{
    template<class RNG>
    auto operator()(RNG&& rng)
    {
        std::vector<
            typename state_flattened_detail::FlattenedValue<RNG>::Type
        > res;
        state_flatten_range_base(std::forward<RNG>(rng), res);
        return res;
    }
};
auto state_flattened()
{
    return StateFlattenTag();
}
template<class RNG>
auto operator|(RNG&& rng, StateFlattenTag t)
{
    return t(std::forward<RNG>(rng));
}
}}
