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
#include <CPT/algorithm/g_m_m_util/access.hpp>
#include <boost/range/combine.hpp>
#include <mlpack/core/metrics/mahalanobis_distance.hpp>
#include <CPT/algorithm/lazy_foreach.hpp>
/* permutation and remove last sample fitting */

namespace cpt {
namespace algorithm {
namespace g_m_m_util {
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace cf = cpt::format;
namespace mm = mlpack::metric;
RANGE_NAMESPACE_SHORTCUT;
template<class METRIC>
struct KMeansHelper{};
template<>
struct KMeansHelper<mm::EuclideanDistance>
{
    template<class OBS, class CONTEXT>
    static auto create_kmeans( OBS&& obs, CONTEXT&& context )
    {
        return mlpack::kmeans::KMeans<
            mm::EuclideanDistance
        >(
            context.max_iter
        );
    }
    template<class OBS, class CONTEXT>
    static auto cov ( OBS&& obs, CONTEXT&& context )
    {
        return arma::Mat<double>(obs.n_rows, obs.n_rows, arma::fill::eye); 
    }
};
template<bool TakeRoot >
struct KMeansHelper<mm::MahalanobisDistance< TakeRoot >>
{
    template<class OBS, class CONTEXT>
    static auto create_kmeans( OBS&& obs, CONTEXT&& context )
    {
        return mlpack::kmeans::KMeans<
            mm::MahalanobisDistance< TakeRoot >
        >(
            context.max_iter
        );
    }
    template<class OBS, class CONTEXT>
    static auto cov ( OBS&& obs, CONTEXT&& context )
    {
        // TODO fix this
        return arma::Mat<double>(obs.n_rows, obs.n_rows, arma::fill::eye); 
    }
};
template<class METRIC>
struct KMeansTrain
{
  public: 
    const std::size_t max_iter;
    template<class T>
    KMeansTrain( const cf::Json<T>& json )
    : max_iter ( json.template get_optional<std::size_t>  ("max_iter").value_or(1000)    ) 
    {} 

    auto operator() (
          arma::mat& obs
        , const std::vector<arma::vec>& ini_means
    ) const
    {
        const std::size_t n_cluster = ini_means.size();
        arma::Row<std::size_t> assm;
        arma::Mat<double> cen( 
              ini_means.size() > 0 ? ini_means.at(0).size() : arma::uword(0)
            , ini_means.size()
        );
        
        for ( auto&& col : ini_means | ::range_indexed(0))
        {
            cen.col(col.index()) = col.value();
        }
        auto kk ( KMeansHelper<METRIC>::create_kmeans(obs, *this) );
        kk.Cluster(
              obs, n_cluster, assm
            , cen, false, true
        );
        std::vector<md::GaussianDistribution> gds;
        arma::Mat<double> cov ( KMeansHelper<METRIC>::cov(obs, *this) );
        for ( auto&& mean : cen | ca::col_range() )
        {
            gds.emplace_back( mean, cov );
        }
        auto one_3 ( 1 / 3.0 );
        return mg::GMM( gds, {one_3, one_3, one_3} );
    }
    decltype(auto) operator() (
          arma::mat& obs
        , const std::vector<md::GaussianDistribution>& ini_dists
    ) const
    {
        auto&& means = ini_dists | ba::transformed(
            []( auto&& gd )
            {
                return gd.Mean();
            }
        ) | ::to_vector;
        return this->operator()( obs, means );
    }
};
struct KMeansClassify
{
    auto distance2( const arma::vec& mean, const arma::vec& smp ) const
    {
        double sum = 0.0;
        for ( auto&& d : smp | ::range_indexed(0) )
        {
            auto tmp = (mean.at(d.index()) - d.value() );
            sum += (tmp * tmp);
        }
        return sum;
    }
    auto score_impl( const mg::GMM& model, const arma::vec& smp ) const
    {
        double min = std::numeric_limits<double>::max();
        std::size_t mini = -1;
        for ( auto&& meani : model | meaned() | ::range_indexed(0))
        {
            auto&& mean = meani.value();
            double sum = distance2( mean, smp );
            if ( sum < min )
            {
                min = sum;
                mini = meani.index();
            }
        }
        assert( mini != std::size_t(-1) );
        return std::make_tuple( mini, min );
    }
    auto score ( const mg::GMM& model, const arma::vec& smp ) const
    {
        return std::get<1>( score_impl ( model, smp ) );
    }
    auto operator() ( 
          const mg::GMM& model
        , const arma::mat& obs
    ) const
    {
        arma::Row<std::size_t> assm(obs.n_cols);
        for ( auto&& coli : obs | ca::col_range() | ::range_indexed(0))
        { 
            auto&& col = coli.value().eval();
            auto tag_score ( score_impl ( model, col ) );
            assm.at(coli.index()) = std::get<0>(tag_score);
        }
        return assm;
    }
};

}}}
