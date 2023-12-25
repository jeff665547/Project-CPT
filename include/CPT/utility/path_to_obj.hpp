#pragma once
#include <string>
#include <fstream>
namespace cpt { 
namespace utility {

template<class T>
struct PathToObj
{
    // static T get( const std::string& path )
    // {}
    static T get( const T& path )
    {
        return path;
    }
    template<class O>
    static T get( const O& path )
    {
        throw std::logic_error("bad path to object called.");
    }
};
#define CREATE_PATH_TO_OBJ( T, stmt )               \
    template<> struct PathToObj<T>                  \
    {                                               \
        template<class O>                           \
        static T get( const O& path )               \
        {                                           \
            throw std::logic_error(                 \
                "bad path to object called.");      \
        }                                           \
        static T get( const std::string& path )     \
        stmt                                        \
    };
CREATE_PATH_TO_OBJ( std::ifstream, { return std::ifstream(path); })

}}
