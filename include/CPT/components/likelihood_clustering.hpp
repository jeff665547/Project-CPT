#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/gaussian_likelihood_clustering.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class LikelihoodClustering : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

  protected:

    // virtual void config_parameters( const bpt::ptree& p ) override
    // {
    // }

  public:

    using Base::Base;

    // virtual void initialize() override
    // {
    // }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component Likelihood Clustering", 2 );
        monitor.log( "Component Likelihood Clustering", "Start ... " );

        std::vector< std::pair< double, double >> xy_datas;
        std::vector< std::pair< size_t, size_t >> xy_datas_index;
        //                        i       j

        monitor.set_monitor( "Converter", db.cube.n_cols + 1 );

        for( size_t j = 0; j < db.cube.n_cols; ++j )
        {
            monitor.log( "Converter", "Converting ... " );

            for( size_t i = 0; i < db.cube.n_rows; ++i )
            {
                xy_datas.emplace_back( std::make_pair( db.cube.at( i, j, 0 ), db.cube.at( i, j, 1 )));
                xy_datas_index.emplace_back( i, j );
            }
        }

        monitor.log( "Converter", "Converting ... Complete" );

        monitor.set_monitor( "Sort Data", xy_datas.size() +4 );
        monitor.log( "Sort Data", "Sorting ... " );

        std::vector< size_t > xy_datas_reorder_indexs( xy_datas.size() );
        std::iota( xy_datas_reorder_indexs.begin(), xy_datas_reorder_indexs.end(), 0 );

        std::sort( xy_datas_reorder_indexs.begin(), xy_datas_reorder_indexs.end(),
            [ &xy_datas ]( const size_t& a, const size_t& b )
            {
                return xy_datas[a].first < xy_datas[b].first;
            }
        );

        monitor.log( "Sort Data", "Sorting ... " );

        std::sort( xy_datas.begin(), xy_datas.end(),
            [ &xy_datas ]( const std::pair< double, double >& a, const std::pair< double, double >& b )
            {
                return a.first < b.first;
            }
        );

        monitor.log( "Sort Data", "Sorting ... " );

        std::vector< double > x_datas( xy_datas.size() );

        for( size_t i = 0; i < xy_datas.size(); ++i )
        {
            monitor.log( "Sort Data", "Sorting ... " );

            x_datas[ i ] = xy_datas[ i ].first;
        }

        cpt::algorithm::GaussianLikelihoodClustering< double > mlc( x_datas );

        monitor.log( "Sort Data", "Sorting ... Complete" );
        monitor.set_monitor( "Cluster", 0 );

        while( true )
        {
            monitor.log( "Cluster", "Sweeping ... " );

            if( !mlc.sweeping() )
            {
                monitor.log( "Cluster", "Sweeping ... Complete" );

                break;
            }
        }

        monitor.set_monitor( "Likelihood", 3 );
        monitor.log( "Likelihood", "Get Maximum Likelihood ..." );

        //           ml boundary_1 boundary_2
        std::tuple< double, size_t, size_t > maximum_item = mlc.get_maximum_likelihood();

        // size_t maximum_likelihood = std::get< 0 >( maximum_item );
        size_t boundary_1 = std::get< 1 >( maximum_item );
        size_t boundary_2 = std::get< 2 >( maximum_item );

        monitor.log( "Likelihood", "Get Maximum Likelihood ... Complete" );

        monitor.set_monitor( "Labelz", xy_datas.size() +2 );
        monitor.log( "Labelz", "Get Labelz ... " );

        {
            cpt::format::Cube< double > cube( db.cube.n_rows, db.cube.n_cols, 1 );
            db.labelz_cube.swap( cube );
        }

        for( size_t idx = 0; idx < xy_datas.size(); ++idx )
        {
            monitor.log( "Labelz", "Get Labelz ... " );

            size_t i = xy_datas_index[ xy_datas_reorder_indexs[ idx ]].first;
            size_t j = xy_datas_index[ xy_datas_reorder_indexs[ idx ]].second;

            if( idx < boundary_1 )
            {
                db.labelz_cube.at( i, j, 0 ) = 0;
            }

            if( idx >= boundary_1 && idx < boundary_2 )
            {
                db.labelz_cube.at( i, j, 0 ) = 1;
            }

            if( idx >= boundary_2 )
            {
                db.labelz_cube.at( i, j, 0 ) = 2;
            }
        }

        monitor.log( "Labelz", "Get Labelz ... Complete" );
        monitor.log( "Component Likelihood Clustering", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
