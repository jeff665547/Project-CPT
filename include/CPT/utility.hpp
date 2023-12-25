#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <Nucleona/util.hpp>
namespace cpt {

template<class T>
void clear( T& v )
{
    v.clear();
}

template<class T>
void clear( std::vector<T>& v )
{
    std::vector<T> blank;
    v.swap(blank);
}

template<class T>
void reset_vector( 
      std::vector<T>&   vec
    , const size_t&     n_cluster
    , const T&          val 
)
{
    vec.resize(n_cluster);
    std::fill( vec.begin(), vec.end(), val );
}
using nucleona::indent;
// std::string indent(int il)
// {
//     static const std::string ele = "    ";
//     std::string res;
//     for ( int i = 0; i < il; i ++ )
//         res += ele;
//     return res;
// }
std::ofstream make_ofstream ( const boost::filesystem::path& fname );

template<class STR>
auto make_ofstream ( const STR& fname )
{
    return make_ofstream( fname );
}
}
#define NOINLINE __attribute__ ((noinline))
