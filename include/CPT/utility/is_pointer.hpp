#pragma once
#include <type_traits>
namespace cpt{ namespace utility{

template<class T> struct IsNakedPointer : public std::false_type {};

template<class T> struct IsNakedPointer< T* > : public std::true_type {};
template<class T> using  IsNakedPointerDecay =  IsNakedPointer< std::decay_t< T > >;


}}
