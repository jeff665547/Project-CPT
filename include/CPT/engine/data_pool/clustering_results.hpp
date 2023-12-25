#pragma once
#include <CPT/forward.hpp>
#include <CPT/engine/data_pool.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mlpack/core.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class ClusteringResults
{
  public:
    arma::Mat<int>          igenotypes;
    arma::Mat<size_t>       genotypes;
    arma::Mat<double>       likelihoods;
    arma::Mat<double>       posteriors;
};

} // end of data_pool
} // end of engine
} // end of cpt
