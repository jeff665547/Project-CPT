#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/data_loader2.hpp>
#else
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CPT/engine/components/named_component.hpp>
#include <CPT/format/cube.hpp>
#include <CPT/logger.hpp>
#include <mlpack/core.hpp>
#include <CPT/format/json.hpp>

namespace cpt {
namespace component {
namespace cf = cpt::format;
namespace cu = cpt::utility;
class DataLoader : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    std::vector< std::string > sample_vec_;
    std::shared_ptr< std::vector< cpt::format::Tsv<> >> tsvs_;

    bool is_tsv_;

    // size_t file_job_number_;
    // size_t file_thread_num_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        using TypeAlias = decltype(tsvs_)::element_type;
        tsvs_ = db.require_shared<TypeAlias>( "DataLoader" );

        /* config thread pool */
        // auto json = cf::make_json( p );
        // db.thread_pool->resize_pool(json.get_optional<std::size_t>("thread_num").value_or(20));

        is_tsv_ = p.get_optional< bool >( "is_tsv" ).value_or( false );

        // file_job_number_ = p.get_optional< size_t >( "file_job_number" ).value_or(  0 );
        // file_thread_num_ = p.get_optional< size_t >( "file_thread_num" ).value_or( 16 );
        db.thread_pool->resize_pool( p.get_optional< size_t >( "thread_num" ).value_or( 16 ));
    }

  public:

    using Base::Base;

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );
        db.start_shared_object_management(); // TODO : deprecate this
        db.initial_shared_data_manager();
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component Data Loader", 4 );
        monitor.log( "Component Data Loader", "CDF ... " );

        db.require_cdf( db.get_path( "chip_layout" ).string() );

        if( db.probeset_table.empty() )
        {
            monitor.set_monitor( "Probeset Table", 2 );
            monitor.log( "Probeset Table", "Build Up ..." );

            for( auto& unit : db.cdf.units )
            {
                cpt::engine::data_pool::Probeset probeset( unit );
                db.probeset_table.push_back( probeset );
            }

            db.num_probes = db.cdf.num_cols * db.cdf.num_rows;
            db.num_probesets = db.probeset_table.size();

            monitor.log( "Probeset Table", "Build Up ... Complete" );
        }

        monitor.log( "Component Data Loader", "CEL ... " );

        if( db.raw_sample_cube.empty() )
        {
            std::vector< bfs::path > paths = db.get_path_list( "sample_files" );

            // switch( file_job_number_ )
            // {
            //     case 0:
            //         file_job_number_ = ( paths.size() / file_thread_num_ ) +1;
            // }
            size_t file_job_number_ = ( paths.size() / db.thread_pool->get_thread_num() ) +1;

            std::mutex file_mutex;
            // ParaThreadPool file_parallel_pool( file_thread_num_ );

            monitor.set_monitor( "Sample File", 2 );
            monitor.log( "Sample File", "Loading ... " );

            cpt::format::Cube< double > cube( db.num_probes, db.num_samples, 2 );
            db.raw_sample_cube.swap( cube );

            size_t sample_count( 0 );
            std::map< bfs::path, size_t > path_pool;

            for( auto& path : paths )
            {
                if( path_pool.find( path ) == path_pool.end() )
                {
                    path_pool.emplace( path, sample_count );
                    sample_vec_.push_back( get_sample_name( path.string() ));
                    sample_count++;
                }
            }

            db.raw_sample_cube.y_axis.set_labels( sample_vec_ );
            std::map< bfs::path, size_t > path_jobs;
            size_t job_id = 0;

            for( auto& path : path_pool )
            {
                job_id++;
                path_jobs.emplace( path );

                if( path_jobs.size() < file_job_number_ )
                {
                    if( job_id != path_pool.size() )
                    {
                        continue;
                    }
                }

                // file_parallel_pool.job_post( [ &db, path_jobs, &file_mutex ] ()
                db.thread_pool->job_post( [ &db, path_jobs, &file_mutex ] ()
                {
                    auto&& data_loader_rh = db.get_holder(
                        "data_loader", cu::ThreadDurationLabel::parallel_mode
                    );
                    std::map<
                        size_t,
                        std::vector< std::vector< float >>
                    > sample_datas;

                    for( auto& path : path_jobs )
                    {
                        auto cel = CELFile::load( path.first.string() );
                        std::vector< std::vector< float >> probe_vecs( 2, std::vector< float >() );

                        for( size_t channel = 0; channel != 2; ++channel )
                        {
                            probe_vecs[ channel ] = cel->extract_intensities( channel );
                        }

                        sample_datas.emplace( path.second, probe_vecs );
                    }

                    {
                        std::lock_guard< std::mutex > file_lock( file_mutex );

                        for( auto& sample : sample_datas )
                        {
                            for( size_t channel = 0; channel != sample.second.size(); ++channel )
                            {
                                for( size_t idx = 0; idx < sample.second[ channel ].size(); ++idx )
                                {
                                    db.raw_sample_cube( idx, sample.first, channel ) = sample.second[ channel ][ idx ];
                                }
                            }
                        }
                    }

                });

                path_jobs.clear();
            }

            // file_parallel_pool.flush_pool();
            db.thread_pool->flush_pool();
            monitor.log( "Sample File", "Loading ... Complete" );
        }

        monitor.log( "Component Data Loader", "TSV ... " );

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

        monitor.log( "Component Data Loader", "Complete!!!" );
    }

    std::string get_sample_name( const std::string& path )
    {
        std::vector< std::string > path_file;
        boost::iter_split( path_file, path, boost::algorithm::first_finder( "/" ));

        std::vector< std::string > sample;
        boost::iter_split( sample, path_file[ path_file.size()-1 ], boost::algorithm::first_finder( "." ));

        return sample[0];
    }

    void setup_tsv()
    {
        // return;

        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "raw_data" );
        std::vector< bfs::path > paths;

        for( auto& sample : sample_vec_ )
        {
            cpt::format::Tsv<> tmp;
            tmp.register_column( "probe_id" );
            tmp.register_column( "channel_0" );
            tmp.register_column( "channel_1" );
            tsvs_->push_back(std::move(tmp));

            auto path = db.output_dir();
            path += sample + ".DataLoader.tsv";

            db.add_output_path( node, path );
            paths.push_back( path );
        }

        db.assign_path( "raw_data", paths );
    }

    void loadup_tsv()
    {
        auto& db( this->mut_data_pool() );
        auto& num_samples = db.raw_sample_cube.n_cols;
        auto& num_probes  = db.raw_sample_cube.n_rows;

        for( size_t j = 0; j != num_samples; ++j )
        {
            for( size_t i = 0; i != num_probes; ++i )
            {
                tsvs_->at( j ).push_entry(
                    std::to_string( i ),
                    db.raw_sample_cube( i, j, 0 ),
                    db.raw_sample_cube( i, j, 1 )
                );
            }
        }
    }

    void res_output()
    {
        // return;

        auto& db( this->mut_data_pool() );

        if( !db.exist_path_tag( "raw_data" ))
        {
            throw std::runtime_error( "data_path_poll not found: \"raw_data\"" );
        }

        auto& paths = db.get_path_list( "raw_data" );

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

} // end of namespace component
} // end of namespace cpt
#endif
