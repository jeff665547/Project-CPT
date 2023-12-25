#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/allele_summarization2.hpp>
#else
#include <CPT/engine/components/named_component.hpp>
#include <CPT/format/tsv.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class AlleleSummarization : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    std::string probeset_list_;
    std::set< std::string > selected_probesets_;
    std::shared_ptr< std::vector< cpt::format::Tsv<> >> tsvs_;

    bool is_tsv_;

    // size_t probesets_job_number_;
    // size_t probesets_thread_num_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        tsvs_ = db.require_shared< std::vector< cpt::format::Tsv<> >>( "AlleleSummarization" );

        probeset_list_ = p.get_optional< std::string >( "probeset_list" ).value_or( "" );

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
        if( probeset_list_ == "" || probeset_list_.empty() )
        {
            return;
        }

        if( !bfs::exists( probeset_list_ ))
        {
            throw std::runtime_error( "Error \"probeset_list\" file path in allele summarization components\n" );
        }

        load_selected_probesets();
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto&& allele_summarization_rh = db.get_holder(
            "allele_summarization", cu::ThreadDurationLabel::iterative_mode
        );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component Allele Summarization", 3 );
        monitor.log( "Component Allele Summarization", "Start ... " );

        cpt::format::Cube< double > cube( db.num_probesets, db.num_samples, 2 );

        if( !selected_probesets_.empty() )
        {
            cpt::format::Cube< double > cube_temp( selected_probesets_.size(), db.num_samples, 2 );
            cube.swap( cube_temp );
        }

        // switch( probesets_job_number_ )
        // {
        //     case 0:
        //         probesets_job_number_ = ( selected_probesets_.size() / probesets_thread_num_ ) +1;
        // }
        size_t probesets_job_number_ = ( selected_probesets_.size() / db.thread_pool->get_thread_num() ) +1;

        std::mutex probesets_mutex;
        // ParaThreadPool probesets_parallel_pool( probesets_thread_num_ );

        std::vector< std::string > probeset_vec( db.probeset_table.size(), "" );
        std::map< size_t, size_t > jobs;

        bool is_cube = true;
        size_t cube_id = 0;

        for( size_t table_id = 0; table_id < db.probeset_table.size(); ++table_id )
        {
            is_cube = true;

            if( !selected_probesets_.empty() &&
                 selected_probesets_.find( db.probeset_table[ table_id ].probeset_id ) == selected_probesets_.end()
              )
            {
                is_cube = false;
            }

            if( is_cube )
            {
                jobs.emplace( table_id, cube_id );
                cube_id++;
            }

            if( jobs.size() < probesets_job_number_ )
            {
                if( table_id != db.probeset_table.size()-1 )
                {
                    continue;
                }
            }

            // probesets_parallel_pool.job_post( [ &db, &probeset_vec, &cube, &probesets_mutex, jobs, this ] ()
            db.thread_pool->job_post( [ &db, &probeset_vec, &cube, &probesets_mutex, jobs, this ] ()
            {
                std::map< size_t, std::string > probeset_map;
                std::map< size_t, std::vector< double >> channel_a;
                std::map< size_t, std::vector< double >> channel_b;

                double sum_a = 0.0;
                double sum_b = 0.0;

                std::vector< std::tuple< size_t, size_t, size_t, double >> cube_temps;

                for( auto& id : jobs )
                {
                    probeset_map.emplace( id.second, db.probeset_table[ id.first ].probeset_id );

                    for( size_t idx = 0; idx < db.probeset_table[ id.first ].probes.size(); ++idx )
                    {
                        for( size_t j = 0; j < db.num_samples; ++j )
                        {
                            if( db.probeset_table[ id.first ].alleles[ idx ] == 'A' )
                            {
                                channel_a[j].push_back( db.cube( 
                                    db.probeset_table[ id.first ]
                                        .probes[ idx ]
                                    , j
                                    , db.probeset_table[ id.first ]
                                        .channels[ idx ]
                                ));
                            }

                            if( db.probeset_table[ id.first ].alleles[ idx ] == 'B' )
                            {
                                channel_b[j].push_back( db.cube( 
                                    db.probeset_table[ id.first ]
                                        .probes[ idx ]
                                    , j
                                    , db.probeset_table[ id.first ]
                                        .channels[ idx ]
                                ));
                            }
                        }
                    }

                    for( size_t j = 0; j != db.num_samples; ++j )
                    {
                        sum_a = 0.0;
                        sum_b = 0.0;

                        for( size_t i = 0; i < channel_a[j].size(); ++i )
                        {
                            sum_a += channel_a[j][i];
                        }

                        for( size_t i = 0; i < channel_b[j].size(); ++i )
                        {
                            sum_b += channel_b[j][i];
                        }

                        if( sum_a != 0 )
                        {
                            cube_temps.emplace_back( id.second, j, 0, sum_a / channel_a[j].size() );
                        }

                        if( sum_b != 0 )
                        {
                            cube_temps.emplace_back( id.second, j, 1, sum_b / channel_b[j].size() );
                        }
                    }

                    channel_a.clear();
                    channel_b.clear();
                }

                {
                    std::lock_guard< std::mutex > probesets_lock( probesets_mutex );

                    for( auto& probeset_id : probeset_map )
                    {
                        probeset_vec[ probeset_id.first ] = probeset_id.second;
                    }

                    for( auto& tuple : cube_temps )
                    {
                        cube( std::get<0>( tuple )
                            , std::get<1>( tuple )
                            , std::get<2>( tuple )
                          ) = std::get<3>( tuple );
                    }
                }
            });

            jobs.clear();
        }

        // probesets_parallel_pool.flush_pool();
        db.thread_pool->flush_pool();

        if( is_tsv_ )
        {
            monitor.set_monitor( "TSV File", 4 );
            monitor.log( "TSV File", "Seting ..." );

            setup_tsv();

            monitor.log( "TSV File", "Loading ..." );

            for( size_t i = 0; i < cube.n_rows; ++i )
            {
                for( size_t j = 0; j < cube.n_cols; ++j )
                {
                    tsvs_->at( j ).push_entry( probeset_vec[ i ], cube( i, j, 0 ), cube( i, j, 1 ), 0, 0.0 );
                }
            }

            monitor.log( "TSV File", "Outputing ..." );

            res_output();

            monitor.log( "TSV File", "Outputing ... Complete" );
        }

        const auto& sample_vec = db.cube.y_axis.get_labels();

        db.cube.swap( cube );

        db.cube.fill_axis_name( "probesets", "samples", "alleles" );
        db.cube.x_axis.set_labels( probeset_vec );
        db.cube.y_axis.set_labels( sample_vec );
        db.cube.z_axis.set_labels( std::vector< std::string >({ "allele_a", "allele_b" }));

        monitor.log( "Component Allele Summarization", "Rotate Dimension ... " );

        db.cube.rotate_dimension( std::vector< size_t >({ 2, 1, 0 }));

        monitor.log( "Component Allele Summarization", "Complete!!!" );

    }

    void load_selected_probesets()
    {
        bool header_check = false;
        std::string line;
        std::fstream file( probeset_list_ );

        if( !file.is_open() )
        {
            throw std::runtime_error( "Error \"probeset_list\" file open in allele summarization components\n" );
        }

        while( std::getline( file, line ) )
        {
            if( line.at( 0 ) == '#' )
            {
                /*
                 *  Other header stuff here
                 */
                continue;
            }

            if( header_check )
            {
                selected_probesets_.emplace( line );
                continue;
            }

            if( line == "probeset_id" || line == "probeset_name" )
            {
                header_check = true;
            }
        }

        file.close();
    }

    void setup_tsv()
    {
        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "allele_signals" );
        std::vector< bfs::path > paths;

        for( auto& sample : db.cube.y_axis.get_labels() )
        {
            cpt::format::Tsv<> tsv;
            tsv.register_column( "probeset_id" );
            tsv.register_column( "allele_a" );
            tsv.register_column( "allele_b" );
            tsv.register_column( "genotype" );
            tsv.register_column( "posterior" );
            tsvs_->push_back( std::move( tsv ));

            auto path = db.output_dir();
            path += sample + ".AlleleSummarization.tsv";

            db.add_output_path( node, path );
            paths.push_back( path );
        }

        db.assign_path( "allele_signals", paths );
    }

    void res_output()
    {
        auto& db( this->mut_data_pool() );

        if( !db.exist_path_tag( "allele_signals" ))
        {
            throw std::runtime_error( "data_path_poll not found: \"allele_signals\"" );
        }

        auto& paths = db.get_path_list( "allele_signals" );

        for( size_t i = 0; i < tsvs_->size(); ++i )
        {
            auto& path = paths[i];
            std::ofstream os( path.string() );

            tsvs_->at( i ).ref()
                .select( "probeset_id", "allele_a", "allele_b" )
                .where([]( const auto& o ){ return( true ); })
                .view().dump( os ); 
        }
        tsvs_.reset();
        return;
    }
};

} // end of namespace component
} // end of namespace cpt
#endif
