#pragma once
#include <map>
#include <Nucleona/type_traits/core.hpp>
namespace cpt {
namespace algorithm {
template<bool b, class P, class T>
struct ToMapTypeBase{};
template<class T, class K, class V>
struct ToMapTypeBase<true, std::pair<K, V>, T>
{
    using Type = std::map<K, V>;
};
template<class T>
using ToMapType = typename ToMapTypeBase<
    ::is_sequence_container_v<std::decay_t<T>>
    , typename std::decay_t<T>::value_type    
    , std::decay_t<T>
>::Type;

struct ToMap
{
    template<class T>
    auto operator()(std::initializer_list<T>&& list)
    {
        ToMapType<T> res;
        for ( auto&& p : list )
        {
            res.emplace( std::move(p.first), std::move(p.second) );
        }
        return res;
    }
};
auto to_map()
{
    return ToMap();
}
template<class T>
auto operator|( std::initializer_list<T>&& list, ToMap tm )
{
    return tm(std::move(list));
}

}}
