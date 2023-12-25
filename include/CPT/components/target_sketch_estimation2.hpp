#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/quantile_normalization_for_cube.hpp>
#include <CPT/logger.hpp>
#include <CPT/logger_spdlog.hpp>
#include <CPT/engine/data_pool/component_object_manager.hpp>
#include <CPT/format/json.hpp>

namespace cpt {
namespace component {
namespace cu_ = cpt::utility;
namespace com_ = cpt::engine::data_pool::component_object_manager;
namespace cf_ = cpt::format;
class TargetSketchEstimation : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    size_t scaling_factor_;
    size_t thread_num_;
    /* input */
    com_::ICPObjPtr<std::string>                target_sketch_prefix_;
    com_::ICPObjPtr<cpt::format::Cube<double>>  raw_sample_cube_;

    /* output */
    com_::ICPObjPtr<std::string>                target_sketch_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto json ( cf_::make_json( p ) );
        target_sketch_prefix_ = com_::require_w< std::string > (
            json, "target_sketch_prefix", com_::make_ref_parameter("output_dir")
        );
        target_sketch_ = com_::require_w< std::string > (
            json, "target_sketch", com_::make_ref_parameter("target_sketch")
        );
        raw_sample_cube_ = com_::require_w<cpt::format::Cube<double>>(
            json, "raw_sample_cube", com_::make_ref_parameter("raw_sample_cube")
        );

        thread_num_     = p.get_optional< size_t >( "thread_num.content" )    .value_or( 16 );
        scaling_factor_ = p.get_optional< size_t >( "scaling_factor.content" ).value_or( 50000 );
    }

  public:

    using Base::Base;

    virtual void initialize() override
    {
        /* if user has set the target_sketch node
         * , then the prefix will not taken 
         **/
        auto& db( this->mut_data_pool() );

        db.require_scaling_factor( scaling_factor_ );

        target_sketch_prefix_   ->  initialize();
        target_sketch_          ->  initialize_or( target_sketch_prefix_->get() + "sketch.txt" );
        raw_sample_cube_        ->  initialize();
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        // auto&& target_sketch_estimate_rh = db.get_holder(
        //     "target_sketch_estimate", cu_::ThreadDurationLabel::iterative_mode
        // );
        auto& monitor = db.monitor();
        auto& raw_sample_cube = raw_sample_cube_ -> get();

        monitor.set_monitor( "Component Target Sketch Estimation", 4 );

        monitor.log( "Component Target Sketch Estimation", "Setup Quantile Stuff ... " );
        cpt::algorithm::QuantileNormalizationForCube< double > quantile( raw_sample_cube, db.sketch_table, db.scaling_factor, thread_num_ );
        /* setup quantile class by a raw cube and ref sketch table from datapool */

        monitor.log( "Component Target Sketch Estimation", "Estimate Target Sketch ... " );
        quantile.target_sketch_estimate( db.scaling_factor );   // build one sketch_table for the scaling_factor in the datapool

        monitor.log( "Component Target Sketch Estimation", "Write Target Sketch ... " );
        // db.write_sketch( db.get_path( "target_sketch" ).string() );
        db.write_sketch( target_sketch_->get() );

        monitor.log( "Component Target Sketch Estimation", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
#else
#endif
