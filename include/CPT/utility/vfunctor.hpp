#pragma once
namespace cpt {
namespace utility {

template<class R, class... ARGS>
struct VFunctor
{
    virtual R operator()( ARGS&&... args ) const = 0;
};

}}
