#pragma once
#include <CPT/algorithm/g_m_m_util/fitting.hpp>
#include <CPT/algorithm/g_m_m_util/classify.hpp>
#include <CPT/format/json.hpp>
namespace cpt {
namespace algorithm {
namespace g_m_m_util {
namespace cf = cpt::format;
namespace mg = mlpack::gmm;
class GMMTrain
{
  public:
    const std::size_t       n_init              ;
    const double            tol                 ;
    const std::size_t       max_iter            ;
    const bool              use_existing_model  ;

    template<class T>
    GMMTrain( const cf::Json<T>& json )
    : n_init             ( json.template get_optional<uint16_t>     ("n_init")            .value_or(10)     ) 
    , tol                ( json.template get_optional<double>       ("tol")               .value_or(1e-4)   ) 
    , max_iter           ( json.template get_optional<std::size_t>  ("max_iter")          .value_or(300)    ) 
    , use_existing_model ( json.template get_optional<bool>         ("use_existing_model").value_or(false)  ) 
    {}

    decltype(auto) operator() (
          arma::mat& obs
        , const std::vector<arma::vec>& ini_means
    ) const
    {
        return fitting_by_means(
              obs, n_init, tol, max_iter
            , ini_means, use_existing_model 
        );
    }
    decltype(auto) operator() (
          arma::mat& obs
        , const std::vector<md::GaussianDistribution>& ini_dist
    ) const
    {
        return fitting_by_dists(
              obs, n_init, tol, max_iter
            , ini_dist, use_existing_model 
        );
    }
};
struct GMMClassify
{
    decltype(auto) score( const mg::GMM& mdl, const arma::vec& smp ) const 
    {
        return mdl.Probability(smp);
    }
    decltype(auto) operator() (
          const mg::GMM& model
        , const arma::mat& obs
    ) const
    {
        return classify(model, obs);
    }
};
}}}
