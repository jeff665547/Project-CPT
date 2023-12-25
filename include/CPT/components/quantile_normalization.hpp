#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/quantile_normalization2.hpp>
#else
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/quantile_normalization_for_cube.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {
namespace cu = cpt::utility;
class QuantileNormalization : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    size_t scaling_factor_;
    std::string targe_sketch_;

    std::shared_ptr< std::vector< cpt::format::Tsv<> >> tsvs_;

    bool is_tsv_;

    size_t thread_num_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );

        tsvs_ = db.require_shared< std::vector< cpt::format::Tsv<> >>( "QuantileNormalization" );

        scaling_factor_ = p.get_optional< size_t >( "scaling_factor" ).value_or( 50000 );
        targe_sketch_ = p.get_optional< std::string >( "target_sketch" ).value_or( "" );

        is_tsv_ = p.get_optional< bool >( "is_tsv" ).value_or( false );

        thread_num_ = p.get_optional< size_t >( "thread_num" ).value_or( 16 );
    }

  public:

    using Base::Base;

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );

        if( db.exist_path_tag( "target_sketch" ))
        {
            targe_sketch_ = db.get_path( "target_sketch" ).string();
        }
        else
        {
            // auto path = db.output_dir();
            // path += "sketch.txt";
            // db.push_path( "target_sketch", path );
            std::runtime_error( "target_sketch is need" );
        }

        if( targe_sketch_ != "" )
        {
            db.require_sketch( targe_sketch_ );
        }

        db.require_scaling_factor( scaling_factor_ );
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto&& quantile_normalization_rh = db.get_holder(
            "quantile_normalization", cu::ThreadDurationLabel::iterative_mode
        );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component Quantile Normalization", 5 );
        monitor.log( "Component Quantile Normalization", "Start ... " );

        if( db.cube.empty() )
        {
            cpt::format::Cube< double > cube( db.raw_sample_cube ); // copy a raw cube from datapool
            db.cube.swap( cube ); // swap cube to db.quantiled_cube
        }

        monitor.log( "Component Quantile Normalization", "Setup Quantile Stuff ... " );
        cpt::algorithm::QuantileNormalizationForCube< double > quantile( db.cube, db.sketch_table, db.scaling_factor, thread_num_ );   // setup quantile class by a copy of raw cube and ref sketch table from datapool

        monitor.log( "Component Quantile Normalization", "Get Sketch ... " );
        if( db.sketch_table.empty() )   // if there is no sketch_table in the datapool
        {
            // quantile.target_sketch_estimate( db.scaling_factor );   // build one sketch_table for the scaling_factor in the datapool
            // db.write_sketch( db.get_path( "target_sketch" ).string() );
            std::runtime_error( "target_sketch is need" );
        }

        monitor.log( "Component Quantile Normalization", "Quantile Normalization ... " );
        quantile.do_quantile_normalization();   // do quantile normalization, the result will be the ref cube when quantile class was setup

        db.cube.y_axis.set_labels( db.raw_sample_cube.y_axis.get_labels() );

        if( is_tsv_ )
        {
            monitor.set_monitor( "TSV File", 4 );

            monitor.log( "TSV File", "Setting ..." );

            setup_tsv();

            monitor.log( "TSV File", "Loading ..." );

            loadup_tsv();

            monitor.log( "TSV File", "Outputing ..." );

            res_output();

            monitor.log( "TSV File", "Outputing ... Complete" );
        }

        monitor.log( "Component Quantile Normalization", "Complete!!!" );
    }

    void setup_tsv()
    {
        // return;

        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "quantile_normalization" );
        std::vector< bfs::path > paths;

        for( auto& sample : db.cube.y_axis.get_labels() )
        {
            cpt::format::Tsv<> tsv;
            tsv.register_column( "probe_id" );
            tsv.register_column( "channel_0" );
            tsv.register_column( "channel_1" );
            tsvs_->push_back(std::move(tsv));

            auto path = db.output_dir();
            path += sample + ".QuantileNormalization.tsv";

            db.add_output_path( node, path );
            paths.push_back( path );
        }

        db.assign_path( "quantile_normalization", paths );
    }

    void loadup_tsv()
    {
        auto& db( this->mut_data_pool() );
        auto& num_samples = db.cube.n_cols;
        auto& num_probes  = db.cube.n_rows;

        for( size_t j = 0; j < num_samples; ++j )
        {
            for( size_t i = 0; i < num_probes; ++i )
            {
                tsvs_->at( j ).push_entry(
                    std::to_string( i ),
                    db.cube( i, j, 0 ),
                    db.cube( i, j, 1 )
                );
            }
        }
    }

    void res_output()
    {
        // return;

        auto& db( this->mut_data_pool() );

        if( !db.exist_path_tag( "quantile_normalization" ))
        {
            throw std::runtime_error( "data_path_poll not found: \"quantile_normalization\"" );
        }

        auto& paths = db.get_path_list( "quantile_normalization" );

        for( size_t i = 0; i < tsvs_->size(); ++i )
        {
            auto& path = paths[i];
            std::ofstream os( path.string() );

            tsvs_->at( i ).ref()
                .select( "probe_id", "channel_0", "channel_1" )
                .where([]( const auto& o ){ return( true ); })
                .view().dump( os ); 
        }
        tsvs_.reset();
    }
};

} // namespace component
} // namespace cpt
#endif
