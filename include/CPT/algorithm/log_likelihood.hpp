#pragma once
#include <algorithm>
#include <utility>
#include <mlpack/methods/gmm/gmm.hpp>
#include <armadillo>
#include <boost/range/irange.hpp>
#include <boost_addon/range_indexed.hpp>
#include <CPT/algorithm/mdl_components.hpp>
namespace cpt {
namespace algorithm {
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
template<class COMPS_RNG>
double log_likelihood(
    const arma::mat& data,
    const COMPS_RNG& distsL,
    const arma::vec& weightsL
) 
{
    double loglikelihood = 0;
    arma::vec phis;
    arma::mat likelihoods(boost::distance(distsL), data.n_cols);

    for( const auto& comp : distsL | ::range_indexed(0) )
    {
        comp.value().Probability(data, phis);
        likelihoods.row(comp.index()) 
            = weightsL(comp.index()) * arma::trans(phis);

    }
    // Now sum over every point.
    for (size_t j = 0; j < data.n_cols; j++)
        loglikelihood += log( arma::accu(likelihoods.col(j)));
    return loglikelihood;
}
double log_likelihood( const mg::GMM& gmm, const arma::mat& data )
{
    return log_likelihood(
          data
        , gmm | mdl_components()
        , gmm.Weights()
    );
}

}}
