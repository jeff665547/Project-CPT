#pragma once
#include <iostream>
#include <CPT/logger.hpp>
#include <Nucleona/language.hpp>

namespace cpt{ namespace utility{ 
    auto safe_assert( bool flag )
    {
#ifdef NDEBUG
#else
        if( flag ) return;
        else 
        {
            cpt::dbg << "assert fail at :" << __FILE__ << ':' << __LINE__ << std::endl;
            abort();

        }
#endif
    }
    template<class T, class STR, class LN >
    decltype(auto) safe_assert( T&& value, STR&& file,  LN&& line )
    {
#ifdef NDEBUG
#else
        if( value >= 0 ) return FWD(value);
        else 
        {
            cpt::dbg << "assert fail at :" << file << ':' << line << std::endl;
            abort();

        }
#endif
    }
    template<class EARG = const char*, class E = std::runtime_error>
    inline auto throw_if( bool flag, EARG&& arg = "condition check fail" )
    {
        if ( flag ) throw E( arg );
    }

}}

#define sassert( expr ) cpt::utility::safe_assert( expr, __FILE__, __LINE__ )

