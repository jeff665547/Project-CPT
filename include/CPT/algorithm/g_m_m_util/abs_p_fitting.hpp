#pragma once
#include <utility>
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <Nucleona/container/vector.hpp>
#include <CPT/algorithm/g_m_m_util/fitting.hpp>
#include <CPT/range.hpp>
#include <CPT/algorithm/g_m_m_util/classify.hpp>
#include <CPT/algorithm/sort.hpp>
#include <CPT/algorithm/mdl_components.hpp>
#include <CPT/format/json.hpp>
#include <boost_addon/range_filter.hpp>
#include <CPT/utility/vfunctor.hpp>
#include <CPT/logger.hpp>
/* permutation and remove last sample fitting */
namespace cpt {
namespace algorithm {
namespace g_m_m_util {

namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace cf = cpt::format;
RANGE_NAMESPACE_SHORTCUT;

struct AbsPFitting : public cpt::algorithm::g_m_m_util::FittingAlgo
{
    const double                    min_probability;
    const std::size_t               n_init              ;
    const double                    tol                 ;
    const std::size_t               max_iter            ;
    const bool                      use_existing_model  ;

    template<class T>
    AbsPFitting ( const cf::Json<T>& json )
    : min_probability ( json
        .template get_optional<double>("min_probability").value_or(0.01) 
    )
    , n_init             ( json.template get_optional<uint16_t>     ("n_init")            .value_or(10)    ) 
    , tol                ( json.template get_optional<double>       ("tol")               .value_or(1e-4)  )
    , max_iter           ( json.template get_optional<std::size_t>  ("max_iter")          .value_or(300)   ) 
    , use_existing_model ( json.template get_optional<bool>         ("use_existing_model").value_or(false) ) 
    {}
    virtual mg::GMM operator() ( FITTING_ALGO_INTERFACE ) const override
    {
        auto mdl ( fitting_by_means( 
              obs, n_init, tol, max_iter
            , ini_means, use_existing_model 
        ) );
        auto before_obs_num = obs.n_cols;
        std::vector<arma::vec> pass_sample;
        obs.each_col(
            [&](arma::vec& col)
            {
                auto p = mdl.Probability(col);
                if ( p > min_probability ) 
                {
                    pass_sample.emplace_back(col);
                }
            }
        );
        obs = pass_sample | ca::col_rng_to_arma_mat(obs.n_rows, pass_sample.size());
        if ( before_obs_num != obs.n_cols )
        {
            cpt::logout5 << "secondary fitting run" << std::endl;
            mdl = fitting_by_dists(
                  obs
                , n_init
                , tol
                , max_iter
                , mdl | ca::mdl_components() | to_vector
                , use_existing_model
            );
        }
        return mdl;
    }

}; 
template<
      class ML_TRAIN
    , class ML_CLASSIFY
>
struct AbsPFittingRT 
: public cpt::algorithm::g_m_m_util::FittingAlgo
{
    ML_TRAIN                        ml_train            ;
    ML_CLASSIFY                     ml_classify         ;
    const double                    min_score           ;
    template<class T>
    AbsPFittingRT ( const cf::Json<T>& json )
    : min_score ( json
        .template get_optional<int>("min_score").value_or(3) 
    )
    , ml_train          { json }
    // , ml_classify       { json }
    {}
    virtual mg::GMM operator() ( 
        FITTING_ALGO_INTERFACE
    ) const override
    {
        auto mdl ( ml_train( obs, ini_means ) );
        auto before_obs_num = obs.n_cols;
        std::vector<arma::vec> pass_sample;
        obs.each_col(
            [&](arma::vec& col)
            {
                auto p = ml_classify.score ( mdl, col );
                if ( p > min_score ) 
                {
                    pass_sample.emplace_back(col);
                }
            }
        );
        obs = pass_sample | ca::col_rng_to_arma_mat(obs.n_rows, pass_sample.size());
        if ( before_obs_num != obs.n_cols )
        {
            cpt::logout5 << "secondary fitting run" << std::endl;
            mdl = ml_train( obs, mdl | ca::mdl_components() | to_vector );
        }
        return mdl;
    }
};

}}}
