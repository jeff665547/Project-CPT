#pragma once 
#include <CPT/forward.hpp>
#include <CPT/format/cube.hpp>
#include <Nucleona/format/hdf5.hpp>
#include <iostream>
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <string>
#include <CPT/engine/data_pool.hpp>

namespace cpt {
namespace algorithm {

class Birdseed
{
  public:
    using Model = mlpack::gmm::GMM;

  public:
    static void train(
        const arma::Mat<double>& data  // dims = [num_alleles x num_samples]
      , Model& model
      , const size_t maxiter = 200
      , const double convtol = 1e-4
      , const size_t trials = 5
      , const bool use_existing_model = false
    ) {
        model.Train(data, trials, use_existing_model,
                    mlpack::gmm::EMFit<>(maxiter, convtol));
    }
    static void infer(
        const arma::Mat<double>& data  // dims = [num_alleles x num_samples]
      , const Model& model
      , arma::Row<size_t>& genotypes
      , arma::Row<double>& likelihoods
      , arma::Row<double>& posteriors
    ) {
        model.Classify(data, genotypes);
        for (size_t j = 0; j != data.n_cols; ++j)
        {
            likelihoods(j) = model.Probability(data.col(j));
            posteriors(j) = model.Probability(data.col(j), genotypes(j))
                          / model.Probability(data.col(j));
            // assert(posteriors(j) != 0.0);
        }
    }
};

} // end of namespace algorithm
} // end of namespace cpt
