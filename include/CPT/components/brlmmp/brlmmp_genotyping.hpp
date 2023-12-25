#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

namespace com_ = cpt::engine::data_pool::component_object_manager;
namespace cf_ = cpt::format;

class BRLMMpGenotyping : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    std::string brlmmp_type_;
    std::string output_path_;
    std::string clustering_models_;

    double cutoff_;
    std::size_t thread_num_;

    com_::ICPObjPtr< cf_::Cube<double> > probeset_cube_;

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto& root = db.pipeline_schema();

        brlmmp_type_ = p.get_optional< std::string >( "brlmmp_type.content" ).value_or( "Gaussian" );
        output_path_ = p.get_optional< std::string >( "output_path.content" ).value_or( "" );

        for( auto& child : root.get_child( "context.clustering_models" ))
        {
            clustering_models_ = child.second.get_value< std::string >();
        }

        cutoff_     = p.get_optional< double >( "cut_off.content" ).value_or( 0.66 );
        thread_num_ = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );

        auto json = cf_::make_json( p );
        probeset_cube_ = com_::require_w<cf_::Cube<double>>(
            json, "probeset_cube", com_::make_ref_parameter("probeset_cube")
        );
    }

  public:

    using Base::Base;

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );

        if( output_path_ == "" )
        {
            auto path = db.output_dir();
            path += "genotype.tsv";
            output_path_ = path.string();
        }

        bfs::path output_path = output_path_;
        auto& node = db.require_output( "genotype" );
        db.add_output_path( node, output_path );

        if( db.thread_pool->get_thread_num() != thread_num_ )
        {
            db.thread_pool->resize_pool( thread_num_ );
        }

        probeset_cube_ -> initialize();
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        auto& probeset_cube = probeset_cube_ -> get();
        size_t cube_x = probeset_cube.n_rows;
        size_t cube_y = probeset_cube.n_cols;

        monitor.set_monitor( "Component BRLMMp Genotyping", 6 );
        monitor.log( "Component BRLMMp Genotyping", "Start ... " );

        cpt::algorithm::BRLMMp brlmmp( 3, brlmmp_type_ );

        monitor.log( "Component BRLMMp Genotyping", "Reading Model ... " );

        brlmmp.set_training( clustering_models_ );

        monitor.log( "Component BRLMMp Genotyping", "Reading Model ... Complete" );

        std::vector< std::vector< std::pair< int16_t, double >>>
            genotypings_confidences( cube_x, std::vector< std::pair< int16_t, double >>() );

        monitor.set_monitor( "Genotyping", 2 );
        monitor.log( "Genotyping", "Genotyping ... " );

        std::mutex probesets_mutex;
        std::vector< size_t > jobs;
        size_t task_number = ( cube_x / db.thread_pool->get_thread_num() ) +1;

        for( size_t i = 0; i < cube_x; ++i )
        {
            jobs.emplace_back( i );

            if( jobs.size() < task_number )
            {
                if( i != cube_x-1 )
                {
                    continue;
                }
            }
            
            db.thread_pool->job_post( [ &probeset_cube, &cube_y, &brlmmp, &genotypings_confidences, &probesets_mutex, jobs, this ] ()
            {
                std::vector< std::pair< int16_t, double >> genotyping_confidence;
                std::map< size_t, std::vector< std::pair< int16_t, double >>> genotypings_confidences_map;

                for( auto& id : jobs )
                {
                    for( size_t j = 0; j < cube_y; ++j )
                    {
                        genotyping_confidence.push_back( brlmmp.genotyping( id, probeset_cube( id, j, 0 ), brlmmp.buffers, cutoff_ ));
                    }

                    genotypings_confidences_map.emplace( id, genotyping_confidence );
                    genotyping_confidence.clear();
                }

                {
                    std::lock_guard< std::mutex > probesets_lock( probesets_mutex );

                    for( auto& job : genotypings_confidences_map )
                    {
                        genotypings_confidences[ job.first ] = job.second;
                    }
                }
            }
            );

            jobs.clear();
        }

        db.thread_pool->flush();

        monitor.log( "Genotyping", "Genotyping ... Complete" );
        monitor.log( "Component BRLMMp Genotyping", "Outputing Result ... " );

        std::vector< std::string > probeset_name( probeset_cube.x_axis.get_labels() );
        std::vector< std::string > sample_name( probeset_cube.y_axis.get_labels() );

        std::ofstream output( output_path_ );
        output << "x_axis\ty_axis\tgenotype\tconfidence\tprobeset_name\tsample_name\n";

        for( size_t j = 0; j < cube_y; ++j )
        {
            for( size_t i = 0; i < cube_x; ++i )
            {
                output
                    << probeset_cube( i, j, 0 ) << "\t"
                    << probeset_cube( i, j, 1 ) << "\t"
                    << genotypings_confidences[i][j].first << "\t"
                    << genotypings_confidences[i][j].second << "\t"
                    << probeset_name[i] << "\t"
                    << sample_name[j] << "\n"
                ;
            }
        }

        output.close();

        monitor.log( "Component BRLMMp Genotyping", "Outputing Result ... Complete" );
        monitor.log( "Component BRLMMp Genotyping", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
