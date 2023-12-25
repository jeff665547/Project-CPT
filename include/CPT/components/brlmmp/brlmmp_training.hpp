#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class BRLMMpTraining : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    size_t k_cluster_;
    std::string brlmmp_type_;

  public:

    using Base::Base;

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        k_cluster_   = p.get_optional< size_t      >( "k_cluster"   ).value_or( 3 );
        brlmmp_type_ = p.get_optional< std::string >( "brlmmp_type" ).value_or( "Gaussian" );
    }

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );
        auto path = db.output_dir();

        path += "brlmmp_training.txt";

        db.push_path( "brlmmp_training", path );
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        size_t cube_x = db.cube.n_rows;
        size_t cube_y = db.cube.n_cols;

        monitor.set_monitor( "Component BRLMMp Training", 8 );
        monitor.log( "Component BRLMMp Training", "Start ... " );

        cpt::algorithm::BRLMMp brlmmp( k_cluster_, brlmmp_type_ );

        monitor.set_monitor( "Clustering", cube_x +1 );
        for( size_t i = 0; i < cube_x; ++i )
        {
            monitor.log( "Clustering", "Clustering ... " );

            std::vector< double > x_vec;

            for( size_t j = 0; j < cube_y; ++j )
            {
                x_vec.emplace_back( db.cube( i, j, 0 ));
            }

            std::sort( x_vec.begin(), x_vec.end() );

            brlmmp.find_min_bic( x_vec );
        }
        monitor.log( "Clustering", "Clustering ... Complete" );

        monitor.log( "Component BRLMMp Training", "Building Model ... " );
        brlmmp.build_model( brlmmp.buffers );
        monitor.log( "Component BRLMMp Training", "Building Model ... Complete" );

        monitor.log( "Component BRLMMp Training", "Imputation ... " );
        brlmmp.imputate_1_missing_cluster( brlmmp.buffers );
        brlmmp.imputate_2_missing_cluster( brlmmp.buffers );
        monitor.log( "Component BRLMMp Training", "Imputation ... Complete" );

        monitor.log( "Component BRLMMp Training", "Outputing Model ... " );
        brlmmp.output_training( brlmmp.buffers, db.get_path( "brlmmp_training" ).string() );
        monitor.log( "Component BRLMMp Training", "Outputing Model ... Complete" );

        monitor.log( "Component BRLMMp Training", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
