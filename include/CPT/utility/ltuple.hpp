#pragma once
#include <tuple>
#include <CPT/utility/constexpr_ds/string.hpp>
#include <Nucleona/language.hpp>
#include <iostream>
namespace cpt {
namespace utility {

template<class T_NAME>
struct LTupleHeader
{};
namespace ltuple_detail {
    template<int N>
    struct Index
    {
        template<class...T>
        constexpr static int get(const constexpr_ds::String& label, const std::tuple<T...>& h) 
        {
            if ( label == std::get<N>(h) ) return N;
            else return Index<N - 1>::get(label, h);
        }
    };
    template<>
    struct Index<-1>
    {
        template<class...T>
        constexpr static int get(const constexpr_ds::String& label, const std::tuple<T...>& h) 
        {
            return -1;
        }
    };
    template<class... T>
    constexpr int get_idx ( 
          const constexpr_ds::String& label 
        , const std::tuple<T...>& header
    )
    {
        return ltuple_detail::Index<sizeof...(T) - 1>::get(label, header);
    }

}
template<class T_NAME, class... F>
struct LTuple : public std::tuple<F...>
{
    using Base = std::tuple<F...>;
    using Base::Base;
    static constexpr auto header = LTupleHeader<T_NAME>::get();
    std::string to_stdform_str(int li)
    {
        // TODO not implementation
        throw std::logic_error ( "not implementation" );
    }
    std::string to_colepseform(int li)
    {
        // TODO not implementation
        throw std::logic_error ( "not implementation" );
    }
    std::string content(int li)
    {
        // TODO not implementation
        throw std::logic_error ( "not implementation" );
    }
    inline static std::size_t size() noexcept
    {
        return sizeof...(F);
    }
};
// template<class T_NAME, class... F>
// constexpr auto get(
//       const constexpr_ds::String& label
//     , const LTuple<T_NAME, F...>& tup)
// {
//     typedef LTuple<T_NAME, F...> LTP;
//     return std::get<ltuple_detail::get_idx(label, LTP::header)>(tup);
// }
template<class... STR>
constexpr auto make_ltuple_header(STR&&... str)
{
    return std::make_tuple(constexpr_ds::String(str)...);
}
}}
#define MAKE_LTUPLE_HEADER(H_NAME, LABELS...) \
namespace cpt { \
namespace utility { \
template<> \
struct LTupleHeader<H_NAME> \
{ \
    constexpr static auto get() \
    { \
        return make_ltuple_header(LABELS); \
    } \
}; \
}}

#define LTGET(LABEL, TUPLE) \
std::get<cpt::utility::ltuple_detail::get_idx(LABEL, decltype(TUPLE)::header)>(TUPLE)

