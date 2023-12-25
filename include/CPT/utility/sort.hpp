#pragma once
#include <vector>
#include <algorithm>
#include <numeric>

namespace cpt {
namespace utility {

std::vector< double > sort( std::vector< double >& vec )
{
    std::vector< double > idx( vec.size() );
    std::iota( idx.begin(), idx.end(), 0 );

    std::sort( idx.begin(), idx.end(),
        [ &vec ]( double a, double b)
        {
            return vec[a] < vec[b];
        }
    );

    std::sort( vec.begin(), vec.end() );

    return std::move( idx );
}

template< typename TYPE >
std::vector< size_t > sort( std::vector< TYPE >& vec )
{
    std::vector< size_t > idx( vec.size() );
    std::iota( idx.begin(), idx.end(), 0 );

    std::sort( idx.begin(), idx.end(),
        [ &vec ]( size_t a, size_t b)
        {
            return vec[a] < vec[b];
        }
    );

    std::sort( vec.begin(), vec.end() );

    return std::move( idx );
}

} // end of namespace utility
} // end of namespace cpt
