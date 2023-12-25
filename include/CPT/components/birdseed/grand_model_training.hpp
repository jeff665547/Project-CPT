#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/birdseed/grand_model_training2.hpp>
#else

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

namespace cpt {
namespace component {
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

class GrandModelTraining 
: public grand_model_training_detail::GrandModelTrainingBase
{
    using Base = grand_model_training_detail::GrandModelTrainingBase;
    using DataPoolType = typename Base::DataPoolType;
  public:
    GMMFitting<DataPoolType>                g_m_m_fitting           ;
    ced::Shared<AlleleSignal>               log_allele_min          ;
    ced::Shared<AlleleSignal>               log_allele_max          ;
    ced::Shared<AlleleSignal>               log_allele_avg          ;
    ced::Shared<ProbesetSignal>             log_probeset_signal     ;
    ced::Shared<arma::mat>                  log_probeset_signal_t   ;
    ced::Shared<nucleona::container::ndvector<2, double>>    ini_means               ;

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
        /* require cube */
        auto& db( this->mut_data_pool() );

        db.require      ( log_allele_min        , "LogAlleleMin"        );
        db.require      ( log_allele_max        , "LogAlleleMax"        );
        db.require      ( log_allele_avg        , "LogAlleleAvg"        );
        db.require      ( log_probeset_signal   , "LogProbesetSignal"   );
        db.require      ( log_probeset_signal_t , "LogProbesetSignalT"  );
        db.require      ( ini_means             , "GrandModelIniMeans"  );
        /* simulate data route */
        auto json ( cf::make_json() );
        json.add( "observation", "LogProbesetSignalT" ) ;
        json.add( "ini_means"  , "GrandModelIniMeans" ) ;
        json.add( "g_m_m"      , "GrandModel"         ) ;
        g_m_m_fitting.config_parameters( p );
        g_m_m_fitting.require_list ( json.root );
    }
    static arma::mat log_probeset_signal_transform (
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
         * 1. log_probeset_signal
         * 2. log_allele_min
         * 3. log_allele_max
         * 4. log_allele_avg
         */
        int64_t sum(0);
        bool flag(false);
        {
            int64_t addr = (int64_t)(log_probeset_signal.instance->d);
            if ( addr == 0 ) flag = true;
            sum += addr;
        }
        {
            int64_t addr = (int64_t)(log_allele_min.instance->d);
            if ( addr == 0 ) flag = true;
            sum += addr;
        }
        {
            int64_t addr = (int64_t)(log_allele_max.instance->d);
            if ( addr == 0 ) flag = true;
            sum += addr;
        }
        {
            int64_t addr = (int64_t)(log_allele_avg.instance->d);
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
                ia & log_probeset_signal.instance->d;
                ia & log_allele_min.instance->d;
                ia & log_allele_max.instance->d;
                ia & log_allele_avg.instance->d;
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
            oa & log_probeset_signal.instance->d;
            oa & log_allele_min.instance->d;
            oa & log_allele_max.instance->d;
            oa & log_allele_avg.instance->d;
        }

#endif 
    }
    virtual void initialize() override
    {
        /* TODO load cube */
        parameter_check();
        auto& db( this->mut_data_pool() );
        db.hard_load( 
              log_probeset_signal_t
            , log_probeset_signal_transform( log_probeset_signal.get() )
        );
        data_pool_.release ( log_probeset_signal   )  ;
        g_m_m_fitting.initialize();
    }
    auto make_ini_means() 
    {
        nucleona::container::ndvector<2, double> ini_means_vec;
        {
            std::vector<double> row(2);
            row[0] = log_allele_max.get()[0];
            row[1] = log_allele_min.get()[1];
            ini_means_vec.emplace_back ( std::move( row ) );
        }
        {
            std::vector<double> row(2);
            row[0] = log_allele_avg.get()[0];
            row[1] = log_allele_avg.get()[1];
            ini_means_vec.emplace_back ( std::move( row ) );
        }
        {
            std::vector<double> row(2);
            row[0] = log_allele_min.get()[0];
            row[1] = log_allele_max.get()[1];
            ini_means_vec.emplace_back ( std::move( row ) );
        }
        log_allele_min.release();
        log_allele_max.release();
        log_allele_avg.release();
        return ini_means_vec;
    }
    virtual void start() override
    {
        data_pool_.hard_load(ini_means, make_ini_means());
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component BirdSeed Grand Model Training", 2 );
        monitor.log( "Component BirdSeed Grand Model Training", "Start" );

        // auto&& grand_model_train_rh = db.get_holder(
        //     "birdseed::grand_model_train", cu::ThreadDurationLabel::iterative_mode
        // );

        g_m_m_fitting.start();

        monitor.log( "Component BirdSeed Grand Model Training", "End" );
    }
    virtual void finish() override
    {
        data_pool_.release ( log_probeset_signal_t )  ;
        data_pool_.release ( ini_means             )  ;
        g_m_m_fitting.finish();
    }
    void cache()
    {
    }
};

}}}
#endif
