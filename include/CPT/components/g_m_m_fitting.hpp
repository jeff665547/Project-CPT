#pragma once

#ifdef NEW_DATA_POOL
#include <CPT/components/g_m_m_fitting2.hpp>
#else

#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
#include <vector>
#include <CPT/engine/data_pool/shared_data_manager.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost_addon/range_eval.hpp>
#include <boost_addon/range_sink.hpp>
#include <CPT/format/json.hpp>
#include <utility>
#include <armadillo>
#include <CPT/algorithm/g_m_m_fitting.hpp>
#include <Nucleona/container/vector.hpp>


namespace cpt {
namespace component {
namespace cf    = cpt::format;
namespace cu    = cpt::utility;
namespace ced   = cpt::engine::data_pool;
namespace ba    = boost::adaptors;
namespace ca    = cpt::algorithm;
namespace md    = mlpack::distribution;
namespace mg    = mlpack::gmm;
namespace g_m_m_fitting_detail {
}
template<class DB>
class GMMFitting
{
  protected:
    using GMM = mg::GMM;
    uint16_t        n_components  ;
    uint16_t        n_init        ;
    double          tol           ;
    std::size_t     max_iter      ;
    const DB&       data_pool_    ;
  public:
    ced::Shared<arma::mat>                  observation         ;
    ced::Shared<nucleona::container::ndvector<2, double>>    ini_means           ;
    ced::Shared<GMM>                        g_m_m               ;
    GMMFitting ( const DB& db )
    : data_pool_ ( db )
    {}
    
    DB& mut_data_pool()
    {
        return const_cast<DB&>(data_pool_);
    }
    void config_parameters( const bpt::ptree& p ) 
    {
        auto json ( cf::make_json(p) );
        n_components = json
            .get_optional<uint16_t>   ("n_components")
            .value_or( 3    );

        n_init       = json
            .get_optional<uint16_t>   ("n_init")      
            .value_or( 20   );

        tol          = json
            .get_optional<double>     ("tol")         
            .value_or( 1e-5 );

        max_iter     = json
            .get_optional<std::size_t>("max_itr")     
            .value_or( 500  );
    }
    template<class T>
    void require( ced::Shared<T>& s, const std::string& name )
    { // engine code
        if( name != "none" )
            mut_data_pool().require(s, name );
    }
    void require_list( const bpt::ptree& p )
    {
        auto json ( cf::make_json( p ) );
        require(observation , json.get<std::string>( "observation"   ));
        require(ini_means   , json.get<std::string>( "ini_means"     ));
        require(g_m_m       , json.get<std::string>( "g_m_m"     ));
    }
    void initialize()
    {}
    void start() 
    {
        ca::GMMFitting train;
        data_pool_.hard_load( 
              g_m_m
            , train.call_by_means(
                  observation.get()
                , n_init
                , tol
                , max_iter
                , ini_means.get()
            )
        );
    }
    void finish()
    {
         data_pool_.release(observation);
         data_pool_.release(ini_means  );
         data_pool_.release(g_m_m      );
    }
};

}}
#endif
