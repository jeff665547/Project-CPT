#pragma once
#include <algorithm>
#include <cmath>
#include <armadillo>
#include <CPT/range.hpp>
#include <boost/range/combine.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <CPT/algorithm/bic.hpp>
#include <mlpack/core/metrics/mahalanobis_distance.hpp>
#include <mlpack/core/metrics/lmetric.hpp>
namespace cpt {
namespace algorithm {
namespace dist_constrain {
RANGE_NAMESPACE_SHORTCUT
namespace mg = mlpack::gmm;
namespace mm = mlpack::metric;
struct Bic /* only 2 dimension work */
{
    template<class OB>
    static auto score( const mg::GMM& gmm, OB&& ob)
    {
        return ca::bic ( gmm, ob ) / ob.n_cols;
    }
};

struct GMM
{
    static auto free_degree(const mg::GMM& model)
    {
        auto&& k = model.Gaussians();
        auto&& dim = model.Dimensionality();
        return 
              k
            * ( 
                ( ( 3 + dim ) * dim / 2 )
                + 1
            )
            - 1
        ;
    }
};
template<class Metric> 
struct KMeans{};
template<> 
struct KMeans<mm::EuclideanDistance>
{
    static auto free_degree(const mg::GMM& model)
    {
        auto&& k = model.Gaussians();
        auto&& dim = model.Dimensionality();
        return k * dim;
    }
};
template<bool TakeRoot>
struct KMeans<mm::MahalanobisDistance<TakeRoot>>
{
    static auto free_degree(const mg::GMM& model)
    {
        auto&& k = model.Gaussians();
        auto&& dim = model.Dimensionality();
        return k * ( dim + ( ( 1 + dim ) * dim / 2 ) );
    }
};

template<class M>
struct Bic2
{
    template<class OB>
    static auto score( const mg::GMM& model, OB&& sample)
    {
        auto v = log_likelihood( model, sample );
        return 
              ( ( -2 * v )
            + ( M::free_degree(model) * std::log(sample.n_cols) ) )/ sample.n_cols;
    }
};

}}}
