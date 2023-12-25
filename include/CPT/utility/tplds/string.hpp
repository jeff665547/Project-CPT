#ifndef CPT_UTILITY_TPLDS_STRING_HPP
#define CPT_UTILITY_TPLDS_STRING_HPP

#include <string>

#include <CPT/utility/tplds/algo.hpp>
#include <CPT/utility/tplds/const_value.hpp>
#include <CPT/utility/tplds/t_pack.hpp>

#define STR_EXPEND_128( str, i ) \
    STR_EXPEND_64( str, i ), STR_EXPEND_64( str, i + 64 )
#define STR_EXPEND_64( str, i ) \
    STR_EXPEND_32( str, i ), STR_EXPEND_32( str, i + 32 )
#define STR_EXPEND_32( str, i ) \
    STR_EXPEND_16( str, i ), STR_EXPEND_16( str, i + 16 )
#define STR_EXPEND_16( str, i ) \
    STR_EXPEND_8( str, i ), STR_EXPEND_8( str, i + 8  )
#define STR_EXPEND_8( str, i ) \
    STR_EXPEND_4( str, i ), STR_EXPEND_4( str, i + 4  )
#define STR_EXPEND_4( str, i ) \
    STR_EXPEND_2( str, i ), STR_EXPEND_2( str, i + 2  )
#define STR_EXPEND_2( str, i ) \
    STR_EXPEND_1( str, i ), STR_EXPEND_1( str, i + 1  )
#define STR_EXPEND_1( str, i ) i < sizeof(str) - 1 ? str[i] : 0 
#define STR_EXPEND(str) STR_EXPEND_128(str, 0)
#define STR_LEN_CHECK(str) (sizeof(str) - 1 > 128)

namespace cpt {
namespace utility {
namespace tplds {
template<char c1, char... cn>
struct String
{
    static std::string to_stl_string()
    {
        return std::string { c1, cn... };
    }
    constexpr static std::size_t length = sizeof...(cn) + 1;
    constexpr static char head = c1;
    using Tail = String<cn...>;
};



/* compiling time optimization */
template<char... good, char... o>
struct PackRemoveZeroImpl<char, TPack<char, good...>, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', o...>
{
    using Result = typename PackRemoveZeroImpl<char, TPack<char, good...>, o...>::Result;
};
template<char... good, char... o>
struct PackRemoveZeroImpl<char, TPack<char, good...>, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', o...>
{
    using Result = typename PackRemoveZeroImpl<char, TPack<char, good...>, o...>::Result;
};
template<char... good, char... o>
struct PackRemoveZeroImpl<char, TPack<char, good...>, '\0', '\0', '\0', '\0', o...>
{
    using Result = typename PackRemoveZeroImpl<char, TPack<char, good...>, o...>::Result;
};
template<char... good, char... o>
struct PackRemoveZeroImpl<char, TPack<char, good...>, '\0', '\0', o...>
{
    using Result = typename PackRemoveZeroImpl<char, TPack<char, good...>, o...>::Result;
};
/* compiling time optimiztion end */

template<class T_PACK> struct TPackToStringImpl{};
template<char... o>
struct TPackToStringImpl<TPack<char, o...>>
{
    using Result = String<o...>;
};
template<class T> using TPackToString = typename TPackToStringImpl<T>::Result;
template<bool check, char... o>
struct StringTFactory
{
    using Result = TPackToString<PackRemoveZero<char, o...>>;
};
template<char... o> struct StringTFactory<true, o...>{};
template<bool check, char...o> using TryGetString = typename StringTFactory<check, o...>::Result;

}}}
#define TPL_STR(str) cpt::utility::tplds::TryGetString<STR_LEN_CHECK(str), STR_EXPEND(str)>
#endif
