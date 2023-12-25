#pragma once 
// #include <iostream>
#include <mlpack/methods/gmm/gmm.hpp>
namespace cpt {
namespace algorithm {

class AxiomGT
{
  public:
    using Model = mlpack::gmm::GMM;
    static void infer(
        arma::Row<int>&             genotypes
      , arma::Row<double>&          likelihoods
      , arma::Row<double>&          posteriors
      , const arma::Mat<double>&    data   // dims = [num_alleles x num_samples]
      , const Model&                data_model
      , const int&                  copynumber
      , const double&               ocean
      , const double&               maxconfidence
    ) {
        std::unordered_map<int, int> component2genotype = {{0, 2}, {1, 1}, {2, 0}};
        arma::vec p(4);
        double pmax, psum;
        for(std::size_t col_j = 0; col_j < data.n_cols; col_j++) 
        {
            const arma::vec& obs = data.col(col_j);

            p.zeros();
            p(0) = data_model.Probability(obs, 0);  // BB genotype clusters
            p(1) = data_model.Probability(obs, 1);  // AB genotype clusters
            p(2) = data_model.Probability(obs, 2);  // AA genotype clusters
            if(copynumber < 2)  p(1) = 0;           // No chance of hets if 1 copy

            likelihoods(col_j)          = arma::sum(p);
            arma::uword max_p_component = p.index_max();
            
            pmax = p.max();
            p(3) = ocean;                           // How deep is the uniform ocean relative to these guys
            p   /= pmax;
            psum = arma::sum(p);
            p   /= psum;
            
            posteriors(col_j) = p(max_p_component);
            genotypes (col_j) = component2genotype[max_p_component];

            // No call
            double confidence = 1 - posteriors(col_j);
            if(confidence > maxconfidence) 
            {
                genotypes(col_j) = -1;
            }
            // Debug
            // std::cout <<   posteriors(col_j)   << std::endl;
            // std::cout << 1 - posteriors(col_j) << std::endl;
            // std::cout <<  "Call: " << genotypes (col_j)   << std::endl;
        }
    }
};

} // end of namespace algorithm
} // end of namespace cpt
