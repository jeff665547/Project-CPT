#pragma once

#ifdef NEW_DATA_POOL
#include <CPT/components/birdseed/log_transform2.hpp>
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
#include <CPT/components/birdseed/typedef.hpp>
#include <CPT/algorithm/log_transform.hpp>
#include <CPT/utility/irange.hpp>
#include <CPT/algorithm/matrix_range.hpp>
#include <armadillo>

namespace cpt {
namespace component {
namespace birdseed {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
namespace log_transform_detail {
    using LogTransformBase = engine::NamedComponent;
}

class LogTransform : public log_transform_detail::LogTransformBase
{
    using Base = log_transform_detail::LogTransformBase;
    ced::Shared<ProbesetIds>    probeset_ids ;
    ced::Shared<AlleleSignal>   log_allele_min      ;
    ced::Shared<AlleleSignal>   log_allele_max      ;
    ced::Shared<AlleleSignal>   log_allele_avg      ;
    ced::Shared<ProbesetSignal> probeset_signal     ;
  public:
    template<class... ARGS>
    LogTransform( ARGS&&... args )
    : Base                { std::forward<ARGS>(args)... }
    , probeset_ids        {} 
    , log_allele_min      { "LogAlleleMin"              } 
    , log_allele_max      { "LogAlleleMax"              } 
    , log_allele_avg      { "LogAlleleAvg"              } 
    , probeset_signal     { "LogProbesetSignal"         } 
    {}
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto json ( cf::make_json(p) );
        db.require( 
              probeset_ids
            , json.get_optional<std::string>("probeset_ids")
                .value_or("GrandModelProbesetIds") /* TODO remove this after parameter routing done */ 
        );
    }
    virtual void initialize() override
    {
        /* load cube */
        data_pool_.hard_load( log_allele_min , AlleleSignal( {0.0, 0.0} ))  ;
        data_pool_.hard_load( log_allele_max , AlleleSignal( {0.0, 0.0} ))  ;
        data_pool_.hard_load( log_allele_avg , AlleleSignal( {0.0, 0.0} ))  ;
        data_pool_.hard_load( probeset_signal                            )  ;
    }
    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component Log Transform", 2 );
        monitor.log( "Component Log Transform", "Start" );

        // auto&& log_transform_rh = db.get_holder(
        //     "birdseed::log_transform", cu::ThreadDurationLabel::iterative_mode
        // );

        /* cube inplace log */
        auto& allele_signals = db.cube;
        auto& min = log_allele_min.get();
        auto& max = log_allele_max.get();
        auto& avg = log_allele_avg.get();
        std::size_t avg_num(0);
        min = { 
              std::numeric_limits<double>::max()
            , std::numeric_limits<double>::max()
        };
        max = { 
              std::numeric_limits<double>::min()
            , std::numeric_limits<double>::min()
        };
        auto min_ftr ( ca::make_allele_min ( min ) );
        auto max_ftr ( ca::make_allele_max ( max ) );
        auto avg_ftr ( ca::make_allele_avg ( avg, avg_num ) );
        allele_signals 
        | glue(ba::transformed(
            [] ( auto& elem ) -> auto&
            {
                elem = log( elem );
                return elem;
            }
        ))
        | endp
        ;
        probeset_ids.get()
        | glue(ba::transformed(
            [&allele_signals] ( auto&& pair )
            { 
                return allele_signals
                    .slice ( pair.first  )
                    .col   ( pair.second )
                    .eval()
                ; 
            }
        ))
        | ba::transformed( min_ftr )
        | ba::transformed( max_ftr )
        | ba::transformed( avg_ftr )
        | ::make_range_sink(probeset_signal.get())
        ;
        avg = avg_ftr.get_result();

        monitor.log( "Component Log Transform", "End" );
    }
    virtual void finish() override
    {
        data_pool_.release( log_allele_min      );
        data_pool_.release( log_allele_max      );
        data_pool_.release( log_allele_avg      );
        data_pool_.release( probeset_signal     );
        data_pool_.release( probeset_ids        );
    }
};

}}}
#endif
