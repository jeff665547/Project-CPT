#pragma once
#include <utility>
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <Nucleona/container/vector.hpp>
#include <CPT/algorithm/g_m_m_util/fitting.hpp>
#include <CPT/algorithm/acc_count.hpp>
#include <CPT/range.hpp>
#include <CPT/logger.hpp>
#include <CPT/algorithm/print.hpp>
#include <CPT/utility.hpp>
#include <boost_addon/range_filter.hpp>
#include <CPT/algorithm/set_diff.hpp>
#include <CPT/format/json.hpp>
#include <CPT/algorithm/g_m_m_util/classify.hpp>

/* Outlier Component Removing fitting */
namespace cpt {
namespace algorithm {
namespace g_m_m_util {
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace ca = cpt::algorithm;
namespace cf = cpt::format;
RANGE_NAMESPACE_SHORTCUT

std::size_t get_outlier_threshold ( const mg::GMM& gmm, const arma::mat& ob, double threshold_rate )
{
    double avg ( (double)ob.n_cols / gmm.Gaussians() );
    return avg * threshold_rate;
}
bool remove_outlier(
      mg::GMM               &   model
    , arma::mat             &   ob
    , arma::Row<std::size_t>&   labels
    , double                    threshold_rate
)
{
    std::vector<md::GaussianDistribution> dists;
    std::vector<double> weight;
    std::size_t new_obsize(ob.n_cols);
    std::vector<std::size_t> outlier;
    auto outlier_threshold ( get_outlier_threshold( 
        model, ob, threshold_rate 
    ) );
    for ( auto&& p : ca::acc_count(labels) )
    {
        if ( p.second < outlier_threshold )
        {
            new_obsize -= p.second;
            outlier.push_back ( p.first );
        }
    }
    ob = (
        labels
            | glue(range_indexed(0))
            | glue(make_range_filter(
                [&outlier]( auto&& p )
                {
                    for( auto&& o : outlier )
                    {
                        if ( p.second == o ) return false;
                    }
                    return true;
                }
            )) /* remove outlier sample point index */
            | glue(ba::transformed(
                [&ob]( auto&& p )
                {
                    return ob.col(p.first);
                }
            )) /* get sample point by conserved index */
            | ca::col_rng_to_arma_mat(2, new_obsize)
    )
    ;
    cpt::dbg << "conserved ob : \n" << ob << std::endl;
    if ( ob.n_cols < outlier_threshold ) return false; 

    auto conserved_component_num (
        cu::irange_0(model.Gaussians())
        | ca::set_diff( outlier )
        | ca::printed ( cpt::dbg, ", " )
        | ba::transformed( // store component
            [&dists, &model]( auto&& cid ) 
            { 
                dists.emplace_back(
                    model.Component(cid)
                );
                return cid;
            }
        )
        | ba::transformed( // store model
            [&weight, &model] ( auto&& cid )
            {
                weight.emplace_back( 
                    model.Weights()[cid]
                );
                return cid;
            }
        )
        | ca::eval_distanced
    )
    ;
    cpt::dbg << std::endl;
    auto res = (model.Gaussians() - conserved_component_num) > 0;
    model = mg::GMM(dists, weight);
    return res;
}
template<class OB, class INI_MEANS >
auto o_c_c_fitting(
            OB           &&  obs
    , const std::size_t  &   n_init
    , const double       &   tol
    , const std::size_t  &   max_iter 
    ,       INI_MEANS    &&  ini_means
    , const double       &   threshold_rate
    , const bool         &   use_existing_model = false
)
{
    mg::GMM model;
    while ( true ) /* remove observations until no outlier */
    {
        model = fitting_by_means(
              obs, n_init, tol, max_iter
            , ini_means, use_existing_model 
        );
        auto labels ( classify( model, obs ) );
        if(!remove_outlier( model, obs, labels, threshold_rate )) break;
    }
    return model;
}

struct OCCFitting : public cpt::algorithm::g_m_m_util::FittingAlgo
{
    double                          threshold_rate      ;
    const std::size_t               n_init              ;
    const double                    tol                 ;
    const std::size_t               max_iter            ;
    const bool                      use_existing_model  ;

    template<class T>
    OCCFitting ( const cf::Json<T>& json )
    : threshold_rate ( json
        .template get_optional<double>("threshold_rate").value_or(0.15) 
    )
    , n_init             ( json.template get_optional<uint16_t>     ("n_init")            .value_or(10)     ) 
    , tol                ( json.template get_optional<double>       ("tol")               .value_or(1e-4)   ) 
    , max_iter           ( json.template get_optional<std::size_t>  ("max_iter")          .value_or(300)    ) 
    , use_existing_model ( json.template get_optional<bool>         ("use_existing_model").value_or(false)  ) 
    {}

    virtual mg::GMM operator() ( 
        FITTING_ALGO_INTERFACE
    ) const override
    {
        return o_c_c_fitting(
              obs
            , n_init
            , tol
            , max_iter 
            , ini_means
            , threshold_rate
            , use_existing_model
        );
    }
};
template<
      class ML_TRAIN
    , class ML_CLASSIFY
>
struct OCCFittingRT 
: public cpt::algorithm::g_m_m_util::FittingAlgo
{
    ML_TRAIN                        ml_train            ;
    ML_CLASSIFY                     ml_classify         ;
    double                          threshold_rate      ;

    template<class T>
    OCCFittingRT ( const cf::Json<T>& json )
    : threshold_rate    ( json
        .template get_optional<double>("threshold_rate").value_or(0.15) 
    )
    , ml_train          ( json )
    // , ml_classify       ( json )
    {}

    virtual mg::GMM operator() ( 
        FITTING_ALGO_INTERFACE
    ) const override
    {
        mg::GMM model;
        while ( true ) /* remove observations until no outlier */
        {
            model = ml_train(obs, ini_means);
            auto labels ( ml_classify( model, obs ) );
            if(!remove_outlier( model, obs, labels, threshold_rate )) break;
        }
        return model;
    }
};
}}}
