#pragma once
#include <CPT/format/json.hpp>
#include <CPT/algorithm/g_m_m_util/kmean.hpp>
#include <CPT/algorithm/dist_constrain/bic.hpp>
#include <mlpack/core/metrics/lmetric.hpp>
#include <mlpack/core/metrics/mahalanobis_distance.hpp>
namespace cpt {
namespace component {
namespace birdseed {
namespace ca = cpt::algorithm;
namespace cf = cpt::format;
namespace cagu = ca::g_m_m_util;
namespace cadc = ca::dist_constrain;
namespace mm = mlpack::metric;
template<
      template<class... ARGS> class FittingAlgo
    , template<class... ARGS> class CONSTRAIN
>
struct KMeansAlgoFactory
{
    template<class MET, class LOG>
    static CONSTRAIN<cadc::KMeans<MET>>* get_kmean_dist_constrain(LOG&& logger)
    {
        // return new CONSTRAIN<cadc::GMM>(logger);
        return new CONSTRAIN<cadc::KMeans<MET>>(5, 10, logger);
        // return new DistConstrainT<cadc::GMM>(4, 10, grand_model.get(), logger);
    }
    template<class COMP, class T>
    static auto set_component( COMP&& comp, const cf::Json<T>& algo )
    {
        auto&& subtype_metric = algo
            .template get_optional<std::string>("subtype_metric")
            .value_or("EuclideanDistance");
        if ( subtype_metric == "EuclideanDistance" )
        {
            comp.fitting_algo.reset      ( new FittingAlgo<
                  cagu::KMeansTrain<mm::EuclideanDistance>
                , cagu::KMeansClassify
            >( algo ) );
            comp.dist_constrain.reset    ( 
                get_kmean_dist_constrain<mm::EuclideanDistance>(comp.logger) 
            );
        }
        else if ( subtype_metric == "MahalanobisDistance" )
        {
            comp.fitting_algo.reset      ( new FittingAlgo<
                  cagu::KMeansTrain<mm::MahalanobisDistance<>>
                , cagu::KMeansClassify
            >( algo ) );
            comp.dist_constrain.reset    ( 
                get_kmean_dist_constrain<
                    mm::MahalanobisDistance<>
                >(comp.logger) 
            );
        }
        else
        {
            throw std::logic_error("unknow subtype_metric : " + subtype_metric);
        }
    }
};
}}}
