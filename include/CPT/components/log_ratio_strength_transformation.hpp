#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/transformations.hpp>
#include <CPT/logger.hpp>
#include <CPT/engine/data_pool/component_object_manager.hpp>
#include <CPT/format/json.hpp>

namespace cpt {
namespace component {
namespace com_ = cpt::engine::data_pool::component_object_manager;
namespace cf_ = cpt::format;
class LogRatioStrength : public engine::NamedComponent
{
    using Base = engine::NamedComponent;
    using Tsvs = std::vector< cpt::format::Tsv<> >;

    /* input */
    com_::ICPObjPtr< std::string > mvt_prefix_;

    /* TODO this object is inplace process, but should be disjoin i/o process, and this process need JIT compile tech */
    com_::ICPObjPtr< cpt::format::Cube<double> > probeset_cube_; 

    /* output */
    com_::ICPObjPtr< Tsvs > tsvs_;
    com_::ICPObjPtr< std::vector<std::string> > mvt_allele_signals_; /* output */

    bool is_tsv_;
    std::size_t thread_num_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto json = cf_::make_json(p);
        tsvs_ = com_::require_w< Tsvs >( 
              json, "log_ratio_strength"
            , com_::make_ref_parameter("LogRatioStrength") 
        );
        probeset_cube_ = com_::require_w<cpt::format::Cube<double>>(
            json, "probeset_cube", com_::make_ref_parameter("probeset_cube")
        );

        mvt_prefix_ = com_::require_w< std::string >(
            json, "mvt_prefix", com_::make_ref_parameter("output_dir")
        );
        mvt_allele_signals_ = com_::require_w< std::vector<std::string> >(
              json, "mvt_allele_signals"
            , com_::make_ref_parameter("mvt_allele_signals")
        );

        is_tsv_     = p.get_optional< bool >( "is_tsv.content" ).value_or( false );
        thread_num_ = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );
    }

  public:

    using Base::Base;

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );
        if( db.thread_pool->get_thread_num() != thread_num_ )
        {
            db.thread_pool->resize_pool( thread_num_ );
        }
        tsvs_               -> initialize();
        probeset_cube_      -> initialize();
        mvt_prefix_         -> initialize();
        mvt_allele_signals_ -> initialize();
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();
        auto& probeset_cube = probeset_cube_->get();
        auto& mvt_prefix = mvt_prefix_ -> get();
        auto& mvt_allele_signals = mvt_allele_signals_->get();
        auto& tsvs = tsvs_->get();


        monitor.set_monitor( "Component Log Ratio Strength Transformation", 2 );
        monitor.log( "Component Log Ratio Strength Transformation", "Start ... " );

        probeset_cube.rotate_dimension( std::vector< size_t >({ 2, 1, 0 }));

        cpt::algorithm::Transformations< double > trans( probeset_cube );

        size_t cube_x = probeset_cube.n_rows;
        size_t cube_y = probeset_cube.n_cols;
        size_t task_num = ( cube_x / db.thread_pool->get_thread_num() ) +1;

        std::mutex probesets_mutex;
        // ParaThreadPool probesets_parallel_pool( probesets_thread_num_ );

        std::vector< size_t > jobs;

        for( size_t i = 0; i < cube_x; ++i )
        {
            jobs.emplace_back( i );

            if( jobs.size() < task_num )
            {
                if( i != cube_x-1 )
                {
                    continue;
                }
            }

            // probesets_parallel_pool.job_post( [ &db, &cube_y, &probesets_mutex, &trans, jobs, this ] ()
            db.thread_pool->job_post( [ 
                  &probeset_cube
                , &db
                , &cube_y
                , &probesets_mutex
                , &trans
                , &mvt_prefix
                , jobs
                , this 
            ] ()
            {
                double a = 0;
                double b = 0;

                std::vector< std::tuple< size_t, size_t, double, double >> cube_temps;

                for( auto& id : jobs )
                {
                    for( size_t j = 0; j < cube_y; ++j )
                    {
                        a = probeset_cube( id, j, 0 );
                        b = probeset_cube( id, j, 1 );
                        trans.log_ratio_strength( a, b );
                        cube_temps.emplace_back( id, j, a, b );
                    }
                }

                {
                    std::lock_guard< std::mutex > probesets_lock( probesets_mutex );

                    for( auto& tuple : cube_temps )
                    {
                        probeset_cube( 
                              std::get<0>( tuple )
                            , std::get<1>( tuple ), 0 
                        ) = std::get<2>( tuple );
                        probeset_cube( 
                              std::get<0>( tuple )
                            , std::get<1>( tuple ), 1 
                        ) = std::get<3>( tuple );
                    }
                }
            });

            jobs.clear();
        }

        // probesets_parallel_pool.flush_pool();
        db.thread_pool->flush();

        probeset_cube.z_axis.set_labels( std::vector< std::string >({ "x", "y" }));

        if( is_tsv_ )
        {
            monitor.set_monitor( "TSV File", 4 );
            monitor.log( "TSV File", "Seting ..." );

            setup_tsv( tsvs, mvt_prefix, mvt_allele_signals, probeset_cube );

            monitor.log( "TSV File", "Loading ..." );

            loadup_tsv( tsvs, probeset_cube );

            monitor.log( "TSV File", "Outputing ..." );

            res_output( tsvs, mvt_allele_signals );

            monitor.log( "TSV File", "Outputing ... Complete" );
        }

        monitor.log( "Component Log Ratio Strength Transformation", "Complete!!!" );
    }
    template<class CUBE>
    void setup_tsv(
          Tsvs& tsvs
        , const std::string& prefix
        , std::vector<std::string>& paths 
        , CUBE&& probeset_cube
    )
    {
        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "mvt_transformation" );
        // std::vector< bfs::path > paths;

        for( auto& sample : probeset_cube.y_axis.get_labels() )
        {
            cpt::format::Tsv<> tsv;

            tsv.register_column( "probeset_id" );
            tsv.register_column( "x" );
            tsv.register_column( "y" );

            tsvs.push_back( std::move( tsv ));

            auto path = prefix;
            path += sample + ".LogRatioStrength.tsv";

            db.add_output_path( node, path );
            paths.push_back( path );
        }

        // db.assign_path( "mvt_transformation", paths );
    }
    
    template<class CUBE>
    void loadup_tsv(
          Tsvs& tsvs
        , CUBE&& probeset_cube
    )
    {
        auto& db( this->mut_data_pool() );
        auto& num_samples = probeset_cube.n_cols;
        auto& num_probes  = probeset_cube.n_rows;

        auto& x_axis_labels = probeset_cube.x_axis.get_labels();
        for( size_t j = 0; j != num_samples; ++j )
        {
            for( size_t i = 0; i != num_probes; ++i )
            {
                tsvs.at( j ).push_entry(
                    x_axis_labels[i],
                    probeset_cube( i, j, 0 ), 
                    probeset_cube( i, j, 1 )
                );
            }
        }
    }

    void res_output(Tsvs& tsvs, const std::vector<std::string>& path_list )
    {
        auto& db( this->mut_data_pool() );
        // const auto& sample_vec = db.cube.y_axis.get_labels();
        // auto& paths = db.get_path_list( "mvt_transformation" );

        for( size_t i = 0; i < tsvs.size(); ++i )
        {
            auto& path = path_list[i];
            std::ofstream output( path );

            tsvs.at( i ).ref()
                .select( "probeset_id", "x", "y" )
                .where([]( const auto& o ){ return( true ); })
                .view().dump( output ); 
        }
    }
};

} // end of namespace component
} // end of namespace cpt
#else
#endif
