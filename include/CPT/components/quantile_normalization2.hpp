#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/quantile_normalization_for_cube.hpp>
#include <CPT/logger.hpp>
#include <CPT/engine/data_pool/component_object_manager.hpp>
#include <CPT/format/json.hpp>

namespace cpt {
namespace component {
namespace cu = cpt::utility;
namespace cf_ = cpt::format;
namespace com_ = cpt::engine::data_pool::component_object_manager;
class QuantileNormalization : public engine::NamedComponent
{
    using Base = engine::NamedComponent;
    using Tsvs = std::vector< cpt::format::Tsv<> >;
    bool is_tsv_;
    size_t thread_num_;
    size_t scaling_factor_;

    /* input */ 
    com_::ICPObjPtr<std::string >               quantile_normalization_prefix_      ;
    com_::ICPObjPtr<std::string>                targe_sketch_                       ;
    com_::ICPObjPtr<cpt::format::Cube<double>>  raw_sample_cube_                    ;

    /* output */
    com_::ICPObjPtr<std::vector<std::string>>   quantile_normalization_opath_       ;
    com_::ICPObjPtr<Tsvs>                       tsvs_                               ; 
    com_::ICPObjPtr<cpt::format::Cube<double>>  probeset_cube_                      ;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto json ( cf::make_json ( p ) );

        quantile_normalization_prefix_
            = com_::require_w<std::string> ( 
                  json
                , "quantile_normalization_prefix"
                , com_::make_ref_parameter(
                    "output_dir"
                )
            );
        quantile_normalization_opath_
            = com_::require_w<std::vector<std::string>>(
                  json
                , "quantile_normalization_opath"
                , com_::make_ref_parameter(
                    "quantile_normalization"
                )
            );
        targe_sketch_    = com_::require<std::string> ( 
            json.get_child( "target_sketch" ) 
        );
        tsvs_            = com_::require_w<Tsvs>      ( 
            json, "tsvs", com_::make_ref_parameter ( "QuantileNormalization") 
        );
        raw_sample_cube_ = com_::require_w<cpt::format::Cube<double>> ( 
            json, "raw_sample_cube", com_::make_ref_parameter("raw_sample_cube")
        );
        probeset_cube_   = com_::require_w<cpt::format::Cube<double>> ( 
            json, "probeset_cube", com_::make_ref_parameter("probeset_cube")
        );

        is_tsv_         = p.get_optional< bool >( "is_tsv.content" ).value_or( false );
        thread_num_     = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );
        scaling_factor_ = p.get_optional< size_t >( "scaling_factor.content" ).value_or( 50000 );
    }

  public:

    using Base::Base;
    
    auto create_probeset_cube( const cpt::format::Cube<double>& raw_sample_cube )
    {
        return raw_sample_cube;
    }

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );
        targe_sketch_                   -> initialize();
        tsvs_                           -> initialize();
        quantile_normalization_prefix_  -> initialize();
        quantile_normalization_opath_   -> initialize();
        raw_sample_cube_                -> initialize();
        probeset_cube_                  -> initialize();

        db.require_scaling_factor( scaling_factor_ );
    }
    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        // auto&& quantile_normalization_rh = db.get_holder(
        //     "quantile_normalization", cu::ThreadDurationLabel::iterative_mode
        // );
        auto& monitor = db.monitor();
        auto& tsvs                          = tsvs_                          -> get();
        auto& targe_sketch                  = targe_sketch_                  -> get();
        auto& quantile_normalization_opath  = quantile_normalization_opath_  -> get();
        auto& quantile_normalization_prefix = quantile_normalization_prefix_ -> get();
        auto& raw_sample_cube               = raw_sample_cube_               -> get();
        auto& probeset_cube                 = probeset_cube_                 -> get();

        monitor.set_monitor( "Component Quantile Normalization", 5 );
        monitor.log( "Component Quantile Normalization", "Start ... " );

        db.require_sketch( targe_sketch );
        probeset_cube = create_probeset_cube( raw_sample_cube );

        // **** the cube is a user parameter from config, so I remove the empty condition
        // if( db.cube.empty() )
        // {
        //     cpt::format::Cube< double > cube( raw_sample_cube ); 
        //     /* copy a raw cube from datapool */
        //     db.cube.swap( cube ); 
        //     /* swap cube to db.quantiled_cube */
        // }

        monitor.log( "Component Quantile Normalization", "Setup Quantile Stuff ... " );
        cpt::algorithm::QuantileNormalizationForCube< double > quantile( probeset_cube, db.sketch_table, db.scaling_factor, thread_num_ );   
        /* setup quantile class by a copy of raw cube and ref sketch table from datapool */

        monitor.log( "Component Quantile Normalization", "Get Sketch ... " );
        if( db.sketch_table.empty() )   
        /* if there is no sketch_table in the datapool */
        {
            throw std::runtime_error( "target_sketch is need" );
        }

        monitor.log( "Component Quantile Normalization", "Quantile Normalization ... " );
        quantile.do_quantile_normalization();   
        /* do quantile normalization, the result will be the ref cube when quantile class was setup */

        probeset_cube.y_axis.set_labels( raw_sample_cube.y_axis.get_labels() );

        if( is_tsv_ )
        {
            monitor.set_monitor( "TSV File", 4 );

            monitor.log( "TSV File", "Setting ..." );

            setup_tsv(
                  tsvs, quantile_normalization_prefix
                , quantile_normalization_opath, probeset_cube
            );

            monitor.log( "TSV File", "Loading ..." );

            loadup_tsv(tsvs, probeset_cube);

            monitor.log( "TSV File", "Outputing ..." );

            res_output(tsvs, quantile_normalization_opath);

            monitor.log( "TSV File", "Outputing ... Complete" );
        }
        tsvs_->release();

        monitor.log( "Component Quantile Normalization", "Complete!!!" );
    }
    void setup_tsv(
          Tsvs& tsvs
        , const std::string& quantile_normalization_prefix
        , std::vector<std::string>& quantile_normalization 
        , cpt::format::Cube<double>& probeset_cube
    )
    {
        // return;

        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "quantile_normalization" );

        for( auto& sample : probeset_cube.y_axis.get_labels() )
        {
            cpt::format::Tsv<> tsv;
            tsv.register_column( "probe_id" );
            tsv.register_column( "channel_0" );
            tsv.register_column( "channel_1" );
            tsvs.push_back(std::move(tsv));

            auto path = quantile_normalization_prefix;
            path += sample + ".QuantileNormalization.tsv";

            db.add_output_path( node, path );
            quantile_normalization.push_back( path );
        }
    }
    void loadup_tsv(Tsvs& tsvs, cpt::format::Cube<double>& probeset_cube)
    {
        auto& db( this->mut_data_pool() );
        auto& num_samples = probeset_cube.n_cols;
        auto& num_probes  = probeset_cube.n_rows;

        for( size_t j = 0; j < num_samples; ++j )
        {
            for( size_t i = 0; i < num_probes; ++i )
            {
                tsvs.at( j ).push_entry(
                    std::to_string( i ),
                    probeset_cube( i, j, 0 ),
                    probeset_cube( i, j, 1 )
                );
            }
        }
    }
    void res_output(Tsvs& tsvs, std::vector<std::string>& opath)
    {
        auto& db( this->mut_data_pool() );
        for( size_t i = 0; i < tsvs.size(); ++i )
        {
            auto& path = opath[i];
            std::ofstream os( path );

            tsvs.at( i ).ref()
                .select( "probe_id", "channel_0", "channel_1" )
                .where([]( const auto& o ){ return( true ); })
                .view().dump( os ); 
        }
    }
};

} // namespace component
} // namespace cpt
#else
#endif
