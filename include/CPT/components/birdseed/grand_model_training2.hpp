#pragma once

#ifdef NEW_DATA_POOL

#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
#include <vector>
#include <CPT/format/tsv.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <CPT/utility/allele_signals_loader.hpp>
// #include <CPT/engine/data_pool/shared_data.hpp>
#include <CPT/engine/data_pool/shared_data_manager.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <CPT/algorithm/allele_min.hpp>
#include <CPT/algorithm/allele_max.hpp>
#include <CPT/algorithm/allele_avg.hpp>
#include <boost_addon/range_eval.hpp>
#include <boost_addon/range_sink.hpp>
#include <CPT/components/g_m_m_fitting.hpp>
#include <CPT/components/birdseed/typedef.hpp>
#include <boost_addon/range_indexed.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <CPT/engine/data_pool/component_object_manager.hpp>

namespace cpt { namespace component {
namespace birdseed {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
namespace bt = boost;
namespace grand_model_training_detail {
    using GrandModelTrainingBase = engine::NamedComponent;
}
namespace com_ = cpt::engine::data_pool::component_object_manager;

class GrandModelTraining 
: public grand_model_training_detail::GrandModelTrainingBase
{
    using Base = grand_model_training_detail::GrandModelTrainingBase;
    using DataPoolType = typename Base::DataPoolType;
  public:
    GMMFitting<DataPoolType>                    g_m_m_fitting           ;
    /* input */
    com_::ICPObjPtr<AlleleSignal>               log_allele_min_          ;
    com_::ICPObjPtr<AlleleSignal>               log_allele_max_          ;
    com_::ICPObjPtr<AlleleSignal>               log_allele_avg_          ;
    com_::ICPObjPtr<ProbesetSignal>             log_probeset_signal_     ;

    /* output */
    com_::ICPObjPtr<arma::mat>                  log_probeset_signal_t_   ;
    com_::ICPObjPtr<nucleona::container::ndvector<2, double>>    ini_means_               ;

    template< class... T >
    GrandModelTraining(
          const DataPoolType& data_pool
        , const T&... o
    )
    : Base          ( data_pool, o... )
    , g_m_m_fitting ( data_pool )
    {}
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto json ( cf::make_json (p) );
        /* TODO fix "LogProbesetSignalT" namming */

        log_allele_min_        = com_::require_w <AlleleSignal> ( 
              json
            , "log_allele_min"         
            , com_::make_ref_parameter("LogAlleleMin"      )  
        );
        log_allele_max_        = com_::require_w <AlleleSignal> ( 
              json
            , "log_allele_max"         
            , com_::make_ref_parameter("LogAlleleMax"      )  
        );
        log_allele_avg_        = com_::require_w <AlleleSignal> ( 
              json
            , "log_allele_avg"         
            ,  com_::make_ref_parameter("LogAlleleAvg"      )  
        );
        log_probeset_signal_   = com_::require_w <ProbesetSignal> ( 
              json
            , "log_probeset_signal"    
            , com_::make_ref_parameter("LogProbesetSignal" )  
        );
        log_probeset_signal_t_ = com_::require_w <arma::mat> ( 
              json
            , "log_probeset_signal_t"  
            , com_::make_ref_parameter("LogProbesetSignalT")  
        );
        ini_means_             = com_::require_w <nucleona::container::ndvector<2, double>> ( 
              json
            , "ini_means"
            , com_::make_ref_parameter("GrandModelIniMeans")  
        );

        auto gj ( cf::make_json() );
        auto observation_j = gj.create_child("observation");
        observation_j.add( "type", "ref" );
        observation_j.add( "content", json.get_optional<std::string>("log_probeset_signal_t.content").value_or("LogProbesetSignalT") );

        auto ini_means_j = gj.create_child("ini_means");
        ini_means_j.add( "type", "ref" );
        ini_means_j.add( "content", json.get_optional<std::string>("ini_means.content").value_or("GrandModelIniMeans") );

        auto g_m_m_j = gj.create_child("g_m_m");
        g_m_m_j.add( "type", "ref" );
        g_m_m_j.add( "content", json.get_optional<std::string>("g_m_m.content").value_or("GrandModel") );

        g_m_m_fitting.config_parameters( gj.root );
        // g_m_m_fitting.require_list ( json.root );
    }
    static arma::mat log_probeset_signal_t_ransform (
        const ProbesetSignal& ps
    )
    {
        arma::mat res( 2, ps.size() );
        for ( auto&& p : ps | ::range_indexed(0))
        {
            res(0, p.index()) = p.value()[0];
            res(1, p.index()) = p.value()[1];
        }
        return res;
    }
    void parameter_check()
    {
#ifdef DEBUG_USE_CACHE
        /* 
         * if parameter ready then just use
         * if parameter no ready then try use cache
         * if cache miss then fail
         *
         * we need to check : 
         * 1. log_probeset_signal_
         * 2. log_allele_min_
         * 3. log_allele_max_
         * 4. log_allele_avg_
         */
        int64_t sum(0);
        bool flag(false);
        {
            int64_t addr = (int64_t)(log_probeset_signal_->get_addr());
            if ( addr == 0 ) flag = true;
            sum += addr;
        }
        {
            int64_t addr = (int64_t)(log_allele_min_->get_addr());
            if ( addr == 0 ) flag = true;
            sum += addr;
        }
        {
            int64_t addr = (int64_t)(log_allele_max_->get_addr());
            if ( addr == 0 ) flag = true;
            sum += addr;
        }
        {
            int64_t addr = (int64_t)(log_allele_avg_->get_addr());
            if ( addr == 0 ) flag = true;
            sum += addr;
        }
        const static std::string cache_path = "./grand_model_training.archive";
        if ( flag )
        {
            assert ( sum == 0 );
            // no data ready
            if(bt::filesystem::exists(cache_path))
            {
                std::ifstream fin(cache_path);
                bt::archive::text_iarchive ia ( fin );
                throw std::logic_error("not implement error");
                // ia & log_probeset_signal_.instance->d;
                // ia & log_allele_min_.instance->d;
                // ia & log_allele_max_.instance->d;
                // ia & log_allele_avg_.instance->d;
            }
            else
            {
                throw std::logic_error("parameter not found");
            }
        }
        else
        {
            // all data ready
            std::ofstream fout ( cache_path );
            bt::archive::text_oarchive oa ( fout );
            throw std::logic_error("not implement error");
            // oa & log_probeset_signal_.instance->d;
            // oa & log_allele_min_.instance->d;
            // oa & log_allele_max_.instance->d;
            // oa & log_allele_avg_.instance->d;
        }

#endif 
    }
    auto make_ini_means_(
          const AlleleSignal& log_allele_max
        , const AlleleSignal& log_allele_min
        , const AlleleSignal& log_allele_avg
    ) 
    {
        nucleona::container::ndvector<2, double> ini_means_vec;
        {
            std::vector<double> row(2);
            row[0] = log_allele_max[0];
            row[1] = log_allele_min[1];
            ini_means_vec.emplace_back ( std::move( row ) );
        }
        {
            std::vector<double> row(2);
            row[0] = log_allele_avg[0];
            row[1] = log_allele_avg[1];
            ini_means_vec.emplace_back ( std::move( row ) );
        }
        {
            std::vector<double> row(2);
            row[0] = log_allele_min[0];
            row[1] = log_allele_max[1];
            ini_means_vec.emplace_back ( std::move( row ) );
        }
        return ini_means_vec;
    }
    virtual void initialize() override
    {
        /* TODO load cube */
        // parameter_check();
        auto& db( this->mut_data_pool() );
        log_probeset_signal_    -> initialize(); 
        log_probeset_signal_t_  -> initialize( 
            log_probeset_signal_t_ransform( log_probeset_signal_->get() )
        );
        log_allele_min_         -> initialize(); 
        log_allele_max_         -> initialize(); 
        log_allele_avg_         -> initialize(); 
        auto& log_allele_max = log_allele_max_->get();
        auto& log_allele_min = log_allele_min_->get();
        auto& log_allele_avg = log_allele_avg_->get();
        ini_means_-> initialize(make_ini_means_(
            log_allele_max, log_allele_min, log_allele_avg
        ));
        log_allele_min_->release();
        log_allele_max_->release();
        log_allele_avg_->release();
        g_m_m_fitting.initialize();

    }
    virtual void start() override
    {


        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component BirdSeed Grand Model Training", 2 );
        monitor.log( "Component BirdSeed Grand Model Training", "Start" );

        g_m_m_fitting.start();

        monitor.log( "Component BirdSeed Grand Model Training", "End" );
    }
    virtual void finish() override
    {
        log_probeset_signal_t_->release();
        ini_means_->release();
        g_m_m_fitting.finish();
    }
};

}}}
#else
#endif
