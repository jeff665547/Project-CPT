#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/engine/data_pool/component_object_manager/icp_obj.hpp>
#include <CPT/format/tsv.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class AlleleSummarization : public engine::NamedComponent
{
    using Base = engine::NamedComponent;
    using Tsvs = std::vector< cpt::format::Tsv<> >;

    std::set<std::string> selected_probesets_set_;
    std::vector< std::string > selected_probesets_;

    /* input */
    com_::ICPObjPtr< std::string > probeset_list_;
    com_::ICPObjPtr< std::string > allele_signals_prefix_;

    /* TODO this object is inplace process, but should be disjoin i/o process, and this process need JIT compile tech */
    com_::ICPObjPtr< cpt::format::Cube<double> > probeset_cube_; 

    /* output */
    com_::ICPObjPtr< std::vector<std::string> > allele_signals_; /* output */
    com_::ICPObjPtr< Tsvs > tsvs_; /* output */

    bool is_tsv_;
    std::size_t thread_num_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto json = cf::make_json( p );
        boost::property_tree::ptree ept;
        tsvs_ = com_::require<Tsvs>(
            cpt::format::make_json(
                json.get_child_optional("tsvs").value_or(ept)
            )
        );
        // tsvs_ = db.require_shared< std::vector< cpt::format::Tsv<> >>( "AlleleSummarization" );
        allele_signals_prefix_ = com_::require_w< std::string >(
            json, "allele_signals_prefix", com_::make_ref_parameter("output_dir")
        );
        allele_signals_ = com_::require_w< std::vector<std::string> >(
            json, "allele_signals", com_::make_ref_parameter("allele_signals")
        );
        probeset_list_ = com_::require< std::string >(
            json.get_child("probeset_list")
        );
        probeset_cube_ = com_::require_w<cpt::format::Cube<double>>(
            json, "probeset_cube", com_::make_ref_parameter("probeset_cube")
        );

        is_tsv_     = p.get_optional< bool >( "is_tsv.content" ).value_or( false );
        thread_num_ = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );
    }

  public:

    using Base::Base;
    void probeset_list_check( const std::string& pslist )
    {
        if( pslist == "" || pslist.empty() )
        {
            return;
        }

        if( !bfs::exists( pslist ))
        {
            throw std::runtime_error( "Error \"probeset_list\" file path in allele summarization components\n" );
        }
    }
    template<class NS, class NP>
    cpt::format::Cube<double> create_cube ( const std::set<std::string>& selected_ps, NS num_samples, NP num_probesets )
    {
        if ( !selected_ps.empty() )
        {
            return cpt::format::Cube< double >( selected_ps.size(), num_samples, 2 );
        }
        else
        {
            return cpt::format::Cube< double >( num_probesets, num_samples, 2 );
        }
    }
    virtual void initialize() override
    {
        probeset_list_          -> initialize();
        tsvs_                   -> initialize();
        allele_signals_prefix_  -> initialize();
        allele_signals_         -> initialize();
        probeset_cube_          -> initialize();
        auto& db( this->mut_data_pool() );
        if( db.thread_pool->get_thread_num() != thread_num_ )
        {
            db.thread_pool->resize_pool( thread_num_ );
        }
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        // auto&& allele_summarization_rh = db.get_holder(
        //     "allele_summarization", cu::ThreadDurationLabel::iterative_mode
        // );
        auto& monitor = db.monitor();
        auto& tsvs                  = tsvs_                     ->  get();
        auto& allele_signals_prefix = allele_signals_prefix_    ->  get();
        auto& allele_signals        = allele_signals_           ->  get();
        auto& probeset_list         = probeset_list_            ->  get();
        auto& probeset_cube         = probeset_cube_            ->  get();
        probeset_list_check ( probeset_list );
        load_selected_probesets( probeset_list );

        monitor.set_monitor( "Component Allele Summarization", 3 );
        monitor.log( "Component Allele Summarization", "Start ... " );

        std::map< std::string, size_t > selected_probesets_map;
        for( size_t i = 0; i < selected_probesets_.size(); ++i )
        {
            selected_probesets_map.emplace( selected_probesets_[i], i );
        }

        cpt::format::Cube< double > cube( create_cube ( selected_probesets_set_, db.num_samples, db.num_probesets() ) );

        // cpt::format::Cube< double > cube( db.num_probesets, db.num_samples, 2 );

        // if( !selected_probesets_.empty() )
        // {
        //     cpt::format::Cube< double > cube_temp( selected_probesets_.size(), db.num_samples, 2 );
        //     cube.swap( cube_temp );
        // }

        size_t task_num = ( selected_probesets_.size() / db.thread_pool->get_thread_num() ) +1;

        std::mutex probesets_mutex;

        std::vector< std::string > probeset_vec( db.num_probesets(), "" );
        std::map< size_t, size_t > jobs;

        bool is_cube = true;

        for( size_t table_id = 0; table_id < db.num_probesets(); ++table_id )
        {
            is_cube = true;

            if( !selected_probesets_.empty() &&
                 selected_probesets_set_.find( db.probeset_name( table_id ) ) == selected_probesets_set_.end()
              )
            {
                is_cube = false;
            }

            if( is_cube )
            {
                jobs.emplace( table_id, selected_probesets_map[db.probeset_name( table_id )] );
            }

            if( jobs.size() < task_num )
            {
                if( table_id != db.num_probesets() - 1 )
                {
                    continue;
                }
            }

            // probesets_parallel_pool.job_post( [ &db, &probeset_vec, &cube, &probesets_mutex, jobs, this ] ()
            db.thread_pool->job_post( [ &db, &probeset_vec, &cube, &probesets_mutex, jobs, this, &probeset_cube ] ()
            {
                std::map< size_t, std::string > probeset_map;
                std::map< size_t, std::vector< double >> channel_a;
                std::map< size_t, std::vector< double >> channel_b;

                double sum_a = 0.0;
                double sum_b = 0.0;

                std::vector< std::tuple< size_t, size_t, size_t, double >> cube_temps;

                for( auto& id : jobs )
                {
                    probeset_map.emplace( id.second, db.probeset_name( id.first ) );

                    for( size_t idx = 0; idx < db.num_probes( id.first ); ++idx )
                    {
                        for( size_t j = 0; j < db.num_samples; ++j )
                        {
                            if( db.probe_allele( id.first, idx )== 'A' )
                            {
                                channel_a[j].push_back( probeset_cube( 
                                    db.probe( id.first, idx )
                                    , j
                                    , db.probe_channel( id.first, idx )
                                ));
                            }

                            if( db.probe_allele( id.first, idx ) == 'B' )
                            {
                                channel_b[j].push_back( probeset_cube( 
                                    db.probe( id.first, idx )
                                    , j
                                    , db.probe_channel( id.first, idx )
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
        db.thread_pool->flush();

        if( is_tsv_ )
        {
            monitor.set_monitor( "TSV File", 4 );
            monitor.log( "TSV File", "Seting ..." );

            setup_tsv(tsvs, allele_signals_prefix, allele_signals, probeset_cube);

            monitor.log( "TSV File", "Loading ..." );

            for( size_t i = 0; i < cube.n_rows; ++i )
            {
                for( size_t j = 0; j < cube.n_cols; ++j )
                {
                    tsvs.at( j ).push_entry( probeset_vec[ i ], cube( i, j, 0 ), cube( i, j, 1 ), 0, 0.0 );
                }
            }

            monitor.log( "TSV File", "Outputing ..." );

            res_output(tsvs, allele_signals);

            monitor.log( "TSV File", "Outputing ... Complete" );
            
            tsvs_->release();
        }

        const auto& sample_vec = probeset_cube.y_axis.get_labels();

        probeset_cube.swap( cube );

        probeset_cube.fill_axis_name( "probesets", "samples", "alleles" );
        probeset_cube.x_axis.set_labels( probeset_vec );
        probeset_cube.y_axis.set_labels( sample_vec );
        probeset_cube.z_axis.set_labels( std::vector< std::string >({ "allele_a", "allele_b" }));

        monitor.log( "Component Allele Summarization", "Rotate Dimension ... " );

        probeset_cube.rotate_dimension( std::vector< size_t >({ 2, 1, 0 }));

        monitor.log( "Component Allele Summarization", "Complete!!!" );

    }

    void load_selected_probesets( const std::string& probeset_list )
    {
        bool header_check = false;
        std::string line;
        std::fstream file( probeset_list );

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
                if( selected_probesets_set_.find( line ) == selected_probesets_set_.end() )
                {
                    selected_probesets_.emplace_back( line );
                    selected_probesets_set_.emplace( line );
                }
                continue;
            }

            if( line == "probeset_id" || line == "probeset_name" )
            {
                header_check = true;
            }
        }

        file.close();
    }

    void setup_tsv( 
          Tsvs& tsvs
        , const std::string& allele_signals_prefix
        , std::vector<std::string>& paths 
        , cpt::format::Cube<double>& probeset_cube
    )
    {
        auto& db( this->mut_data_pool() );
        auto& node = db.require_output( "allele_signals" );
        // std::vector< bfs::path > paths;

        for( auto& sample : probeset_cube.y_axis.get_labels() )
        {
            cpt::format::Tsv<> tsv;
            tsv.register_column( "probeset_id" );
            tsv.register_column( "allele_a" );
            tsv.register_column( "allele_b" );
            tsv.register_column( "genotype" );
            tsv.register_column( "posterior" );
            tsvs.push_back( std::move( tsv ));


            auto path = allele_signals_prefix;
            path += sample + ".AlleleSummarization.tsv";

            db.add_output_path( node, path );
            paths.push_back( path );
        }

        // db.assign_path( "allele_signals", paths );
    }

    void res_output(Tsvs& tsvs, const std::vector<std::string>& allele_signals )
    {
        auto& db( this->mut_data_pool() );

        for( size_t i = 0; i < tsvs.size(); ++i )
        {
            auto& path = allele_signals[i];
            std::ofstream os( path );

            tsvs.at( i ).ref()
                .select( "probeset_id", "allele_a", "allele_b" )
                .where([]( const auto& o ){ return( true ); })
                .view().dump( os ); 
        }
        // tsvs_.reset();
        return;
    }
};

} // end of namespace component
} // end of namespace cpt
#else
#endif
