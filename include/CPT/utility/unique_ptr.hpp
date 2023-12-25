#pragma once
#include <memory>
namespace cpt{ namespace utility{

template<class T>
std::unique_ptr<T> make_unique( T* ptr )
{
    return std::unique_ptr<T>( ptr );
}

}}
