#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/contrast_centers_stretch_transformation2.hpp>
#else
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/transformations.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class ContrastCentersStretch : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    double k_;
    std::shared_ptr< std::vector< cpt::format::Tsv<> >> tsvs_;

    bool is_tsv_;

    // size_t probesets_job_number_;
    // size_t probesets_thread_num_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        tsvs_ = db.require_shared< std::vector< cpt::format::Tsv<> >>( "ContrastCentersStretch" );

        k_ = p.get< double >( "k_scale" );

        is_tsv_ = p.get_optional< bool >( "is_tsv" ).value_or( false );

        // probesets_job_number_ = p.get_optional< size_t >( "probesets_job_number" ).value_or(  0 );
        // probesets_thread_num_ = p.get_optional< size_t >( "probesets_thread_num" ).value_or( 40 );

        size_t thread_num = p.get_optional< size_t >( "thread_num" ).value_or( 64 );

        if( db.thread_pool->get_thread_num() != thread_num )
        {
            db.thread_pool->resize_pool( thread_num );
        }
    }

  public:

    using Base::Base;

    virtual void initialize() override
    {
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component Contrast Centers Stretch Transformation", 2 );
        monitor.log( "Component Contrast Centers Stretch Transformation", "Start ... " );

        db.cube.rotate_dimension( std::vector< size_t >({ 2, 1, 0 }));

        cpt::algorithm::Transformations< double > trans( db.cube );
        // trans.ccs( k_ );

        size_t cube_x = db.cube.n_rows;
        size_t cube_y = db.cube.n_cols;

        // switch( probesets_job_number_ )
        // {
        //     case 0:
        //         probesets_job_number_ = ( cube_x / probesets_thread_num_ ) +1;
        // }
        size_t probesets_job_number_ = ( cube_x / db.thread_pool->get_thread_num() ) +1;

        std::mutex probesets_mutex;
        // ParaThreadPool probesets_parallel_pool( probesets_thread_num_ );

        std::vector< size_t > jobs;

        for( size_t i = 0; i < cube_x; ++i )
        {
            jobs.emplace_back( i );

            if( jobs.size() < probesets_job_number_ )
            {
                if( i != cube_x-1 )
                {
                    continue;
                }
            }

            // probesets_parallel_pool.job_post( [ &db, &cube_y, &probesets_mutex, &trans, jobs, this ] ()
            db.thread_pool->job_post( [ &db, &cube_y, &probesets_mutex, &trans, jobs, this ] ()
            {
                double a = 0;
                double b = 0;

                std::vector< std::tuple< size_t, size_t, double, double >> cube_temps;

                for( auto& id : jobs )
                {
                    for( size_t j = 0; j < cube_y; ++j )
                    {
                        a = db.cube( id, j, 0 );
                        b = db.cube( id, j, 1 );
                        trans.contrast_centers_stretch( k_, a, b );
                        cube_temps.emplace_back( id, j, a, b );
                    }
                }

                {
                    std::lock_guard< std::mutex > probesets_lock( probesets_mutex );

                    for( auto& tuple : cube_temps )
                    {
                        db.cube( std::get<0>( tuple ), std::get<1>( tuple ), 0 ) = std::get<2>( tuple );
                        db.cube( std::get<0>( tuple ), std::get<1>( tuple ), 1 ) = std::get<3>( tuple );
                    }
                }
            });

            jobs.clear();
        }

        // probesets_parallel_pool.flush_pool();
        db.thread_pool->flush_pool();

        db.cube.z_axis.set_labels( std::vector< std::string >({ "x", "y" }));

        if( is_tsv_ )
        {
            monitor.set_monitor( "TSV File", 4 );
            monitor.log( "TSV File", "Seting ..." );

            setup_tsv();

            monitor.log( "TSV File", "Loading ..." );

            loadup_tsv();

            monitor.log( "TSV File", "Outputing ..." );

            res_output();

            monitor.log( "TSV File", "Outputing ... Complete" );
        }

        monitor.log( "Component Contrast Centers Stretch Transformation", "Complete!!!" );
    }

    void setup_tsv()
    {
        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "ccs_transformation" );
        std::vector< bfs::path > paths;

        for( auto& sample : db.cube.y_axis.get_labels() )
        {
            cpt::format::Tsv<> tsv;

            tsv.register_column( "probeset_id" );
            tsv.register_column( "x" );
            tsv.register_column( "y" );

            tsvs_->push_back( std::move( tsv ));

            auto path = db.output_dir();
            path += sample + ".ContrastCentersStretch.tsv";

            db.add_output_path( node, path );
            paths.push_back( path );
        }

        db.assign_path( "ccs_transformation", paths );
    }

    void loadup_tsv()
    {
        auto& db( this->mut_data_pool() );
        auto& num_samples = db.cube.n_cols;
        auto& num_probes  = db.cube.n_rows;

        auto& x_axis_labels = db.cube.x_axis.get_labels();
        for( size_t j = 0; j != num_samples; ++j )
        {
            for( size_t i = 0; i != num_probes; ++i )
            {
                tsvs_->at( j ).push_entry(
                    x_axis_labels[i],
                    db.cube( i, j, 0 ), 
                    db.cube( i, j, 1 )
                );
            }
        }
    }

    void res_output()
    {
        auto& db( this->mut_data_pool() );
        // const auto& sample_vec = db.cube.y_axis.get_labels();

        if( !db.exist_path_tag( "ccs_transformation" ))
        {
            throw std::runtime_error( "data_path_poll not found: \"ccs_transformation\"" );
        }

        auto& paths = db.get_path_list( "ccs_transformation" );

        for( size_t i = 0; i < tsvs_->size(); ++i )
        {
            auto& path = paths[i];
            std::ofstream output( path.string() );

            tsvs_->at( i ).ref()
                .select( "probeset_id", "x", "y" )
                .where([]( const auto& o ){ return( true ); })
                .view().dump( output ); 
        }
    }
};

} // end of namespace component
} // end of namespace cpt
#endif
