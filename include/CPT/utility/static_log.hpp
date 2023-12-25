#pragma once
#include <utility>
namespace cpt { namespace utility {

/* example 
 * StaticLog<2, 256>::result ( this will be 8 )
 */
template<std::size_t BASE, std::size_t VALUE>
struct StaticLogImpl
{
    enum {
        result = 1 + StaticLogImpl< BASE, VALUE/BASE >::result
    };
};
template<std::size_t BASE>
struct StaticLogImpl<BASE, 1>
{
    enum {
        result = 0
    };
};
template<std::size_t BASE, std::size_t VALUE>
struct StaticLog
{
    enum {
        result = StaticLogImpl< BASE, VALUE >::result
    };
};
}}
