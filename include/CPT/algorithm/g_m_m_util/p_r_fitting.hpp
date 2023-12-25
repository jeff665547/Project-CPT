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
/* permutation and remove last sample fitting */
namespace cpt {
namespace algorithm {
namespace g_m_m_util {
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace cf = cpt::format;
RANGE_NAMESPACE_SHORTCUT;

template<class OB, class INI_MEANS >
auto p_r_fitting(
            OB           &&  obs
    , const std::size_t  &   n_init
    , const double       &   tol
    , const std::size_t  &   max_iter 
    ,       INI_MEANS    &&  ini_means
    , const int          &   trim_rate
    , const bool         &   use_existing_model = false
)
{
    const auto trim_count ( obs.n_cols / 100 * trim_rate );
    auto mdl ( fitting_by_means( 
          obs, n_init, tol, max_iter
        , ini_means, use_existing_model 
    ) );
    auto ordered_prob_i ( 
        obs
        | ca::col_range()
        | ba::transformed(
            [&mdl] ( const auto& col )
            { return mdl.Probability( col ); }
        )
        | ca::index_sort<std::vector<std::size_t>>()
    )
    ;
    obs = obs 
        | ca::col_range()
        | ca::idx_access(ordered_prob_i) 
        | ca::idx_access(cu::irange_0( obs.n_cols - trim_count ) )
        | ca::col_rng_to_arma_mat( obs.n_rows, obs.n_cols - trim_count )
    ;
    mdl = fitting_by_dists(
          obs
        , n_init
        , tol
        , max_iter
        , mdl | ca::mdl_components() | to_vector
        , use_existing_model
    );
    return mdl;
}

struct PRFitting : public cpt::algorithm::g_m_m_util::FittingAlgo
{
    int                             trim_rate           ;
    const std::size_t               n_init              ;
    const double                    tol                 ;
    const std::size_t               max_iter            ;
    const bool                      use_existing_model  ;

    template<class T>
    PRFitting ( const cf::Json<T>& json )
    : trim_rate ( json
        .template get_optional<int>("trim_rate").value_or(3) 
    )
    , n_init             ( json.template get_optional<uint16_t>     ("n_init")            .value_or(10)    ) 
    , tol                ( json.template get_optional<double>       ("tol")               .value_or(1e-4)  ) 
    , max_iter           ( json.template get_optional<std::size_t>  ("max_iter")          .value_or(300)   ) 
    , use_existing_model ( json.template get_optional<bool>         ("use_existing_model").value_or(false) ) 
    {}

    virtual mg::GMM operator() ( 
        FITTING_ALGO_INTERFACE
    ) const override
    {
        return p_r_fitting(
              obs
            , n_init
            , tol
            , max_iter 
            , ini_means
            , trim_rate
            , use_existing_model
        );
    }
};
template<
      class ML_TRAIN
    , class ML_CLASSIFY
>
struct PRFittingRT 
: public cpt::algorithm::g_m_m_util::FittingAlgo
{
    ML_TRAIN                        ml_train            ;
    ML_CLASSIFY                     ml_classify         ;
    int                             trim_rate           ;

    template<class T>
    PRFittingRT ( const cf::Json<T>& json )
    : trim_rate ( json
        .template get_optional<int>("trim_rate").value_or(3) 
    )
    , ml_train          { json }
    // , ml_classify       { json }
    {}

    virtual mg::GMM operator() ( 
        FITTING_ALGO_INTERFACE
    ) const override
    {
        const auto trim_count ( obs.n_cols / 100 * trim_rate );
        auto mdl ( ml_train( obs, ini_means ) );
        auto ordered_prob_i ( 
            obs
            | ca::col_range()
            | ba::transformed(
                [&mdl, this] ( const auto& col )
                { return ml_classify.score( mdl, col ); }
            )
            | ca::index_sort<std::vector<std::size_t>>()
        )
        ;
        obs = obs 
            | ca::col_range()
            | ca::idx_access(ordered_prob_i) 
            | ca::idx_access(cu::irange_0( obs.n_cols - trim_count ) )
            | ca::col_rng_to_arma_mat( obs.n_rows, obs.n_cols - trim_count )
        ;
        mdl = ml_train(
              obs
            , mdl | ca::mdl_components() | to_vector
        );
        return mdl;
    }
};
}}}
