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
#include <CPT/components/birdseed/typedef.hpp>
#include <CPT/algorithm/log_transform.hpp>
#include <CPT/utility/irange.hpp>
#include <CPT/algorithm/matrix_range.hpp>
#include <armadillo>
#include <CPT/engine/data_pool/component_object_manager.hpp>

namespace cpt {
namespace component {
namespace birdseed {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
namespace com_ = cpt::engine::data_pool::component_object_manager;
namespace log_transform_detail {
    using LogTransformBase = engine::NamedComponent;
}

class LogTransform : public log_transform_detail::LogTransformBase
{
    using Base = log_transform_detail::LogTransformBase;
    /* input */
    com_::ICPObjPtr<ProbesetIds>                probeset_ids_        ;

    /* in/output ( inplace ) */
    com_::ICPObjPtr<cpt::format::Cube<double>>  probeset_cube_       ;

    /* output */
    com_::ICPObjPtr<AlleleSignal>               log_allele_min_      ;
    com_::ICPObjPtr<AlleleSignal>               log_allele_max_      ;
    com_::ICPObjPtr<AlleleSignal>               log_allele_avg_      ;
    com_::ICPObjPtr<ProbesetSignal>             probeset_signal_     ;

  public:
    template<class... ARGS>
    LogTransform( ARGS&&... args )
    : Base                { std::forward<ARGS>(args)... }
    {}

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto json ( cf::make_json(p) );
        probeset_ids_       = com_::require_w<ProbesetIds>( 
            json, "probeset_ids",   com_::make_ref_parameter("GrandModelProbesetIds") 
        );
        log_allele_min_     = com_::require_w<AlleleSignal>( 
            json, "log_allele_min", com_::make_ref_parameter("LogAlleleMin"         ) 
        );
        log_allele_max_     = com_::require_w<AlleleSignal>( 
            json, "log_allele_max", com_::make_ref_parameter("LogAlleleMax"         ) 
        );
        log_allele_avg_     = com_::require_w<AlleleSignal>( 
            json, "log_allele_avg", com_::make_ref_parameter("LogAlleleAvg"         ) 
        );
        probeset_signal_    = com_::require_w<ProbesetSignal>( 
            json, "probeset_signal",com_::make_ref_parameter("LogProbesetSignal"    ) 
        );
        probeset_cube_      = com_::require_w<cpt::format::Cube<double>>(
            json, "probeset_cube",  com_::make_ref_parameter("probeset_cube"        )
        );
    }
    virtual void initialize() override
    {
        /* load cube */
        log_allele_min_  ->  initialize( AlleleSignal( { 0.0, 0.0 } ) );
        log_allele_max_  ->  initialize( AlleleSignal( { 0.0, 0.0 } ) );
        log_allele_avg_  ->  initialize( AlleleSignal( { 0.0, 0.0 } ) );
        probeset_signal_ ->  initialize();
        probeset_cube_   ->  initialize();
        probeset_ids_    ->  initialize();
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
        auto& probeset_cube   = probeset_cube_          ->  get();
        auto& log_allele_min  = log_allele_min_         ->  get();
        auto& log_allele_max  = log_allele_max_         ->  get();
        auto& log_allele_avg  = log_allele_avg_         ->  get();
        auto& probeset_ids    = probeset_ids_           ->  get();
        auto& probeset_signal = probeset_signal_        ->  get();

        std::size_t avg_num(0);
        log_allele_min = { 
              std::numeric_limits<double>::max()
            , std::numeric_limits<double>::max()
        };
        log_allele_max = { 
              std::numeric_limits<double>::min()
            , std::numeric_limits<double>::min()
        };
        auto min_ftr ( ca::make_allele_min ( log_allele_min ) );
        auto max_ftr ( ca::make_allele_max ( log_allele_max ) );
        auto avg_ftr ( ca::make_allele_avg ( log_allele_avg, avg_num ) );

        probeset_cube 
        | glue(ba::transformed(
            [] ( auto& elem ) -> auto&
            {
                elem = log( elem );
                return elem;
            }
        ))
        | endp
        ;

        probeset_ids
        | glue(ba::transformed(
            [&probeset_cube] ( auto&& pair )
            { 
                return probeset_cube
                    .slice ( pair.first  )
                    .col   ( pair.second )
                    .eval()
                ; 
            }
        ))
        | ba::transformed( min_ftr )
        | ba::transformed( max_ftr )
        | ba::transformed( avg_ftr )
        | ::make_range_sink(probeset_signal)
        ;
        log_allele_avg = avg_ftr.get_result();

        monitor.log( "Component Log Transform", "End" );
    }
    virtual void finish() override
    {
        log_allele_min_  -> release()   ;
        log_allele_max_  -> release()   ;
        log_allele_avg_  -> release()   ;
        probeset_signal_ -> release()   ;
        probeset_ids_    -> release()   ;
        probeset_cube_   -> release()   ;
    }
};

}}}
#else
#endif
