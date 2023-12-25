#pragma once
#include <algorithm>
#include <utility>
#include <mlpack/methods/gmm/gmm.hpp>
#include <armadillo>
#include <boost/range/irange.hpp>
#include <CPT/algorithm/log_likelihood.hpp>
namespace cpt {
namespace algorithm {
namespace mg = mlpack::gmm;


namespace bic_detail {
auto free_degree ( const mg::GMM& model )
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
template< class MEANS >
auto g_m_m_free_degree( MEANS&& means )
{
    auto&& k = means.size();
    auto&& dim = k > 0 ? means.at(0).size() : 0;
    return 
          k
        * ( 
            ( ( 3 + dim ) * dim / 2 )
            + 1
        )
        - 1
    ;

}
double bic(const mg::GMM& model, arma::mat& sample)
{
    auto v = log_likelihood( model, sample );
    return 
          ( -2 * v )
        + ( free_degree(model) * std::log(sample.n_cols) );
}

}

template<class MODEL, class SAMPLE>
auto bic ( MODEL&& model, SAMPLE&& sample )
{
    return bic_detail::bic ( 
          std::forward< MODEL  >( model  )
        , std::forward< SAMPLE >( sample )
    );
}

}}
