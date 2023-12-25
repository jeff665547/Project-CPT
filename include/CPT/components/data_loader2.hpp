#pragma once
#ifdef NEW_DATA_POOL
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CPT/engine/components/named_component.hpp>
#include <CPT/format/cube.hpp>
#include <CPT/logger.hpp>
#include <mlpack/core.hpp>
#include <CPT/format/json.hpp>
#include <CPT/engine/data_pool/component_object_manager/icp_obj.hpp>
#include <CPT/format/cen.hpp>
// #include <boost/filesystem/convenience.hpp>
#include <boost/filesystem.hpp>
#include <mutex>
/* 
 * TODO data pool dependent object convert to icp_object type
 **/
namespace cpt {
namespace component {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace com_ = cpt::engine::data_pool::component_object_manager;
class DataLoader : public engine::NamedComponent
{
    using Base = engine::NamedComponent;
    using Tsvs = std::vector< cpt::format::Tsv<> >;
    std::vector< std::string > sample_vec_;

    /* input */
    com_::ICPObjPtr< std::vector<std::string> >     channel_names_;
    com_::ICPObjPtr< std::vector<std::string> >     sample_files_;
    com_::ICPObjPtr< std::string >                  chip_layout_;
    com_::ICPObjPtr< std::string >                  raw_data_prefix_;

    /* output */
    com_::ICPObjPtr< Tsvs >                         tsvs_;
    com_::ICPObjPtr< std::vector<std::string> >     raw_data_opath_;  
    com_::ICPObjPtr< cpt::format::Cube<double> >    raw_sample_cube_; 
    
    bool is_tsv_;
    auto tolower( const std::string& s )
    {
        std::string res;
        res.resize( s.size() );
        std::transform( s.begin(), s.end(), res.begin(), ::tolower );
        return res;
    }
    std::size_t thread_num_;

  protected:

    /* Read json config (pipline.Input:DataLoader & context.*). */
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        /* config thread pool */
        auto json = cf::make_json( p );
        boost::property_tree::ptree ept;
        tsvs_ = com_::require_w<Tsvs>(
            json, "tsvs", com_::make_ref_parameter("DataLoader")
        );/* "DataLoader" */
        channel_names_ = com_::require_w< std::vector<std::string> >( 
            json, "channel_names", com_::make_ref_parameter("channel_names")
        );
        sample_files_ = com_::require_w< std::vector<std::string> >( 
            json, "sample_files", com_::make_ref_parameter("sample_files")
        );
        chip_layout_ = com_::require_w< std::string > (
            json, "chip_layout", com_::make_ref_parameter("chip_layout")
        );
        raw_data_prefix_ = com_::require_w< std::string > (
            json, "raw_data_prefix", com_::make_ref_parameter("output_dir")
        );
        raw_data_opath_ = com_::require_w<std::vector<std::string>>(
            json, "raw_data_opath", com_::make_ref_parameter("raw_data")
        );
        raw_sample_cube_ = com_::require_w<cpt::format::Cube<double>>(
            json, "raw_sample_cube", com_::make_ref_parameter("raw_sample_cube")
        );

        is_tsv_     = p.get_optional< bool >( "is_tsv.content" ).value_or( false );
        thread_num_ = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );
    }
    decltype(auto) load_chip_sample_data( const std::string& name, const std::vector< std::string >& channels )
    {
        static std::mutex chip_samle_mutex;
        boost::filesystem::path p ( name );
        auto ftype( tolower( p.extension().string() ) );
        if ( ftype == ".cel" )
        {
            return cpt::format::CELFile::load( p.string() );
        }
        else if ( ftype == ".cen" || ftype == ".hdf5" )
        {
            std::lock_guard<std::mutex> lock(chip_samle_mutex);
            return cpt::format::cen::File::load( p.string(), channels );
        }
        else
        {
            throw std::logic_error( "unknow file extension" );
        }
    }
  public:

    using Base::Base;
    
    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );
        com_::CPSymTab::initialize();
        tsvs_               -> initialize();
        channel_names_      -> initialize();
        sample_files_       -> initialize();
        chip_layout_        -> initialize();
        raw_data_prefix_    -> initialize();
        raw_data_opath_     -> initialize();
        raw_sample_cube_    -> initialize();
        db.thread_pool      -> resize_pool( thread_num_ );
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();
        auto& tsvs              = tsvs_             -> get();
        auto& channels          = channel_names_    -> get();
        auto& paths             = sample_files_     -> get();
        auto& chip_layout       = chip_layout_      -> get();
        auto& raw_data_prefix   = raw_data_prefix_  -> get();
        auto& raw_data_opath    = raw_data_opath_   -> get();
        auto& raw_sample_cube   = raw_sample_cube_  -> get();

        monitor.set_monitor( "Component Data Loader", 4 );
        monitor.log( "Component Data Loader", "CDF ... " );

        db.read_chip_layout( db, chip_layout );

        monitor.set_monitor( "Probeset Table", 2 );
        monitor.log( "Probeset Table", "Build Up ..." );

        // db.num_probes = db.chip_layout_num_cols() * db.chip_layout_num_rows();
        // db.num_probesets = db.num_probesets();

        monitor.log( "Probeset Table", "Build Up ... Complete" );

        monitor.log( "Component Data Loader", "CEL ... " );

        if( raw_sample_cube.empty() )
        {
            std::size_t task_num = ( paths.size() / db.thread_pool->get_thread_num() ) +1;

            std::mutex file_mutex;

            monitor.set_monitor( "Sample File", 2 );
            monitor.log( "Sample File", "Loading ... " );

            cpt::format::Cube< double > cube( db.num_probes(), db.num_samples, 2 );
            raw_sample_cube.swap( cube );

            size_t sample_count( 0 );
            std::map< std::string, size_t > path_pool;

            for( auto& path : paths )
            {
                if( path_pool.find( path ) == path_pool.end() )
                {
                    path_pool.emplace( path, sample_count );
                    sample_vec_.push_back( get_sample_name( path ));
                    sample_count++;
                }
            }

            raw_sample_cube.y_axis.set_labels( sample_vec_ );
            std::map< std::string, size_t > path_jobs;
            size_t job_id = 0;

            for( auto& path : path_pool )
            {
                job_id++;
                path_jobs.emplace( path );

                if( path_jobs.size() < task_num )
                {
                    if( job_id != path_pool.size() )
                    {
                        continue;
                    }
                }

                db.thread_pool->job_post( [ &db, path_jobs, &file_mutex, &channels, &raw_sample_cube,this ] ()
                {
                    // auto&& data_loader_rh = db.get_holder(
                    //     "data_loader", cu::ThreadDurationLabel::parallel_mode
                    // );
                    std::map<
                        size_t,
                        std::vector< std::vector< float >>
                    > sample_datas;

                    for( auto& path : path_jobs )
                    {
                        auto data = this->load_chip_sample_data( path.first, channels );
                        std::vector< std::vector< float >> probe_vecs( 2, std::vector< float >() );

                        for( size_t channel = 0; channel != 2; ++channel )
                        {
                            probe_vecs[ channel ] = data->extract_intensities( channel );
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
                                    raw_sample_cube( idx, sample.first, channel ) = sample.second[ channel ][ idx ];
                                }
                            }
                        }
                    }

                });

                path_jobs.clear();
            }

            // file_parallel_pool.flush_pool();
            db.thread_pool->flush();
            monitor.log( "Sample File", "Loading ... Complete" );
        }

        monitor.log( "Component Data Loader", "TSV ... " );

        if( is_tsv_ )
        {
            monitor.set_monitor( "TSV File", 4 );

            monitor.log( "TSV File", "Seting ..." );

            setup_tsv(tsvs, raw_data_prefix, raw_data_opath);

            monitor.log( "TSV File", "Loading ..." );

            loadup_tsv(tsvs, raw_sample_cube);

            monitor.log( "TSV File", "Outputing ..." );

            res_output(tsvs, raw_data_opath);

            monitor.log( "TSV File", "Outputing ... Complete" );
        }
        tsvs_->release();

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

    void setup_tsv(Tsvs& tsvs, const std::string& raw_data_prefix, std::vector<std::string>& raw_data )
    {
        // return;

        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "raw_data" );

        for( auto& sample : sample_vec_ )
        {
            cpt::format::Tsv<> tmp;
            tmp.register_column( "probe_id" );
            tmp.register_column( "channel_0" );
            tmp.register_column( "channel_1" );
            tsvs.push_back(std::move(tmp));

            auto path = raw_data_prefix;
            path += sample + ".DataLoader.tsv";

            db.add_output_path( node, path );
            raw_data.push_back( path );
        }
    }

    void loadup_tsv(Tsvs& tsvs, cpt::format::Cube<double>& raw_sample_cube )
    {
        auto& db( this->mut_data_pool() );
        auto& num_samples = raw_sample_cube.n_cols;
        auto& num_probes  = raw_sample_cube.n_rows;

        for( size_t j = 0; j != num_samples; ++j )
        {
            for( size_t i = 0; i != num_probes; ++i )
            {
                tsvs.at( j ).push_entry(
                    std::to_string( i ),
                    raw_sample_cube( i, j, 0 ),
                    raw_sample_cube( i, j, 1 )
                );
            }
        }
    }

    void res_output(Tsvs& tsvs, std::vector<std::string>& raw_data )
    {
        // return;

        auto& db( this->mut_data_pool() );

        // if( !db.exist_path_tag( "raw_data" ))
        // {
        //     throw std::runtime_error( "data_path_pool not found: \"raw_data\"" );
        // }

        // auto& paths = db.get_path_list( "raw_data" );

        for( size_t i = 0; i < tsvs.size(); ++i )
        {
            auto& path = raw_data[i];
            std::ofstream os( path );

            tsvs.at( i ).ref()
                .select( "probe_id", "channel_0", "channel_1" )
                .where([]( const auto& o ){ return( true ); })
                .view().dump( os );
        }

        // tsvs.reset();
    }
};

} // end of namespace component
} // end of namespace cpt
#else
#endif
