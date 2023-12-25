#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/target_sketch_estimation2.hpp>
#else
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/quantile_normalization_for_cube.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class TargetSketchEstimation : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    size_t scaling_factor_;
    // size_t probesets_thread_num_;
    size_t thread_num_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        scaling_factor_ = p.get_optional< size_t >( "scaling_factor" ).value_or( 50000 );
        // probesets_thread_num_ = p.get_optional< size_t >( "probesets_thread_num" ).value_or( 40 );
        thread_num_ = p.get_optional< size_t >( "thread_num" ).value_or( 16 );
    }

  public:

    using Base::Base;

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );

        db.require_scaling_factor( scaling_factor_ );

        auto path = db.output_dir();
        path += "sketch.txt";
        db.push_path( "target_sketch", path );
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto&& target_sketch_estimate_rh = db.get_holder(
            "target_sketch_estimate", cu::ThreadDurationLabel::iterative_mode
        );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component Target Sketch Estimation", 4 );

        monitor.log( "Component Target Sketch Estimation", "Setup Quantile Stuff ... " );
        // cpt::algorithm::QuantileNormalizationForCube< double > quantile( db.raw_sample_cube, db.sketch_table, scaling_factor_, probesets_thread_num_ );   // setup quantile class by a raw cube and ref sketch table from datapool
        cpt::algorithm::QuantileNormalizationForCube< double > quantile( db.raw_sample_cube, db.sketch_table, scaling_factor_, thread_num_ );   // setup quantile class by a raw cube and ref sketch table from datapool

        monitor.log( "Component Target Sketch Estimation", "Estimate Target Sketch ... " );
        quantile.target_sketch_estimate( db.scaling_factor );   // build one sketch_table for the scaling_factor in the datapool

        monitor.log( "Component Target Sketch Estimation", "Write Target Sketch ... " );
        db.write_sketch( db.get_path( "target_sketch" ).string() );

        monitor.log( "Component Target Sketch Estimation", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
#endif
