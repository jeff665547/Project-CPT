#pragma once
#include <CPT/algorithm/dist_constrain/bic.hpp>
#include <CPT/algorithm/dist_constrain/fan.hpp>
#include <algorithm>
#include <cmath>
#include <armadillo>
#include <CPT/range.hpp>
#include <boost/range/combine.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <set>
namespace cpt {
namespace algorithm {
namespace dist_constrain {
RANGE_NAMESPACE_SHORTCUT
namespace mg = mlpack::gmm;

template<class T, class... CON>
struct MixConstrainIntersection
{
    // TODO find general solution
};
template< class T >
struct MixConstrainIntersection< T, Bic, Fan<> >
{
    // std::set<int64_t> bic_cset;
    auto push_back ( const T* const o )
    {

    }
};

}}}
