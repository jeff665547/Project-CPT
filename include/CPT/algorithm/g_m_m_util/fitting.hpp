#pragma once
#include <utility>
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <Nucleona/container/vector.hpp>
#include <CPT/utility/vfunctor.hpp>
#include <memory>
#include <armadillo>
#include <CPT/algorithm/g_m_m_util.hpp>
namespace cpt {
namespace algorithm {
namespace g_m_m_util {

namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace cu = cpt::utility;

namespace fitting_detail {
auto n_cols( const nucleona::container::ndvector<2, double>& m )
{
    assert( m.size() > 0 );
    return m[0].size();
};
auto n_cols( const arma::mat& m )
{
    return m.n_cols;
}
auto n_cols( const std::vector<arma::vec>& m )
{
    assert(m.size() > 0);
    return m[0].size();
}
auto n_rows( const nucleona::container::ndvector<2, double>& m )
{
    return m.size();
};
auto n_rows( const arma::mat& m )
{
    return m.n_rows;
}
auto n_rows( const std::vector<arma::vec>& m )
{
    return m.size();
}
auto i_matrix(std::size_t n )
{
    arma::mat i_m;
    i_m.eye(n, n);
    return i_m;
}
template<class ELE, class NUM>
auto create_repl_vec(ELE&& ele, NUM&& num)
{
    return std::vector<std::decay_t<ELE>>(
          std::forward<NUM>(num) 
        , std::forward<ELE>(ele)
    );
}
}
namespace fd = fitting_detail;

template<class OB, class GL>
auto fitting_by_dists( 
            OB           &&  obs
    , const std::size_t  &   n_init
    , const double       &   tol
    , const std::size_t  &   max_iter 
    ,       GL           &&  gmlist
    , const bool         &   use_existing_model = false
) 
{
    auto _1_n ( 1.0 / gmlist.size() ); /* all gms are same weight */
    auto weight_vec ( fd::create_repl_vec ( _1_n, gmlist.size() ) );
    mg::GMM gmm ( 
          std::forward<GL> ( gmlist )
        , weight_vec
    );
    gmm.Train( 
          std::forward<OB>( obs )
        , n_init
        , use_existing_model
        , mg::EMFit<>( max_iter, tol )
    );
    return gmm;
}
template<class OB, class INI_MEANS>
auto fitting_by_means( 
            OB           &&  obs
    , const std::size_t  &   n_init
    , const double       &   tol
    , const std::size_t  &   max_iter 
    ,       INI_MEANS    &&  ini_means
    , const bool         &   use_existing_model = false
)
{
    assert ( fd::n_rows(obs) == fd::n_cols(ini_means) ); // dimension check
    auto i_m ( fd::i_matrix ( fd::n_cols( ini_means ) ) );
    std::vector<md::GaussianDistribution> gmlist;
    gmlist.reserve(ini_means.size());
    for ( auto& mean : ini_means )
    {
        gmlist.emplace_back(
              arma::vec(mean)
            , std::move(i_m)
        );
    }
    return fitting_by_dists(
          std::forward<OB>(obs)
        , n_init
        , tol
        , max_iter
        , std::move(gmlist)
        , use_existing_model
    );
}
#define FITTING_ALGO_ARG                     \
            arma::mat                &       \
    , const std::vector<arma::vec>   &       \

#define FITTING_ALGO_INTERFACE                             \
            arma::mat              &  obs                  \
    , const std::vector<arma::vec> &  ini_means            \

using FittingAlgo = cu::VFunctor<
    mg::GMM, FITTING_ALGO_ARG
>;
using FittingAlgoPtr = std::unique_ptr<FittingAlgo>;
}}}
