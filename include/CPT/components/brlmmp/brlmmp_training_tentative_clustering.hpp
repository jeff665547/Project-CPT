#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/brlmmp/brlmmp_training_tentative_clustering2.hpp>
#else
#include <CPT/engine/components/named_component.hpp>
// #include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>
#include <mutex>
#include <functional>
#include <CPT/utility/gprofiler.hpp>
#include <CPT/utility.hpp>
#include <CPT/algorithm/brlmmp_clustering.hpp>

namespace cpt {
namespace component {
namespace cu = cpt::utility;
namespace ca = cpt::algorithm;
namespace brlmmp_detail {
using ResType = std::pair<double, ca::BRLMMpBufferType>;
using IMethod = ca::IBRLMMpClustering<ResType>;
}

template<class T>
auto make_brlmmp_method()
{
    return [](ca::BRLMMpTbl& tbl)
    {
        return std::unique_ptr<brlmmp_detail::IMethod>( new T{tbl});
    };
}
class BRLMMpTrainingTentativeClustering : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    size_t k_cluster_;
    std::string brlmmp_type_;

    // size_t cluster_job_number_;
    // size_t cluster_thread_num_;

  public:

    using Base::Base;
    // template<class... ARGS>
    std::function<
        std::unique_ptr<
            brlmmp_detail::IMethod
        >(ca::BRLMMpTbl& tbl)
    > get_clustering_method( const std::string& subtype)//, ARGS&&... args )
    {
        using ResType = brlmmp_detail::ResType;
        if ( subtype == "Euclidean" )
        {
            return make_brlmmp_method<ca::EuclideanBRLMMp<ResType>> ();
        }
        else if ( subtype == "Gaussian" )
        {
            return make_brlmmp_method<ca::GaussianBRLMMp<ResType>> ();
        }
        else if ( subtype == "Mahalanobis" )
        {
            return make_brlmmp_method<ca::MahalanobisBRLMMp<ResType>>();
        }
        else
        {
            throw std::runtime_error( "Error for brlmmp_type : " + brlmmp_type_ + " in BRLMMpClustering::run()" );
        }
    }
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        k_cluster_   = p.get_optional< size_t      >( "k_cluster"   ).value_or( 3 );
        brlmmp_type_ = p.get_optional< std::string >( "brlmmp_type" ).value_or( "Euclidean" );

        // cluster_job_number_ = p.get_optional< size_t >( "cluster_job_number" ).value_or(  0 );
        // cluster_thread_num_ = p.get_optional< size_t >( "cluster_thread_num" ).value_or( 40 );

        auto& db( this->mut_data_pool() );
        size_t thread_num = p.get_optional< size_t >( "thread_num" ).value_or( 64 );

        if( db.thread_pool->get_thread_num() != thread_num )
        {
            db.thread_pool->resize_pool( thread_num );
        }
    }

    virtual void initialize() override
    {
    }

    virtual void NOINLINE start() override
    {
        cu::GProfiler profiler("BRLMMpTrainingTentativeClustering_start_2");
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        size_t cube_x = db.cube.n_rows; /* probeset */
        size_t cube_y = db.cube.n_cols; /* sample */

        monitor.set_monitor( "Component BRLMMp Training Tentative Clustering", 2 );
        monitor.log( "Component BRLMMp Training Tentative Clustering", " ... " );

        // switch( cluster_job_number_ )
        // {
        //     case 0:
        //         cluster_job_number_ = ( cube_x / cluster_thread_num_ ) +1;
        // }
        size_t cluster_job_number_ = ( cube_x / db.thread_pool->get_thread_num() ) +1;

        std::mutex cluster_mutex;
        // ParaThreadPool cluster_parallel_pool( cluster_thread_num_ );

        std::map<                       // pre Probeset
            size_t,                     // id -> i
            std::vector<                // pre Cluster
                std::pair<
                      double            // each Cluster Likelihood
                    , cpt::algorithm::BRLMMpBufferType  // each Cluster Stuff
                >
            >
        >  probeset_likelihood_clusters;

        db.probeset_likelihood_clusters =
            std::vector<                    // pre Probeset
                std::vector<                // pre Cluster
                    std::pair<
                          double            // each Cluster Likelihood
                        , cpt::algorithm::BRLMMpBufferType  // each Cluster Stuff
                    >
                >
            >( cube_x )
        ;

        std::vector< size_t > cube_xs;

        auto clustering_method_pre = get_clustering_method( brlmmp_type_);
        for( size_t i = 0; i < cube_x; ++i ) // TODO blockwise parallel
        {
            cube_xs.push_back( i );

            if( cube_xs.size() < cluster_job_number_ )
            {
                if( i != cube_x-1 )
                {
                    continue;
                }
            }

            // cluster_parallel_pool.job_post( [ &clustering_method_pre, cube_xs, &db, &cube_y, &cluster_mutex, &probeset_likelihood_clusters, this ] ()
            db.thread_pool->job_post( [ &clustering_method_pre, cube_xs, &db, &cube_y, &cluster_mutex, &probeset_likelihood_clusters, this ] ()
            {
                // TODO thread_local
                std::map<                       // pre Probeset
                    size_t,                     // id -> i
                    std::vector<                // pre Cluster
                        std::pair<
                              double            // each Cluster Likelihood
                            , cpt::algorithm::BRLMMpBufferType  // each Cluster Stuff
                        >
                    >
                > probeset_likelihood_clusters_temp;

                std::vector< double > x_vec;
                std::vector< std::pair< double, cpt::algorithm::BRLMMpBufferType >> likelihood_clusters;
                std::pair< double, cpt::algorithm::BRLMMpBufferType > cluster_temp;

                ca::BRLMMpTbl brlmmp_table;
                for( size_t id = 0; id < cube_xs.size(); ++id )
                {
                    for( size_t j = 0; j < cube_y; ++j )
                    {
                        x_vec.emplace_back( db.cube( cube_xs[ id ], j, 0 ));
                    }

                    std::sort( x_vec.begin(), x_vec.end() );

                    for( size_t k = 0; k < k_cluster_; ++k )
                    {
                        auto clustering_method ( clustering_method_pre( brlmmp_table ) );
                        clustering_method->reset(x_vec, k + 1);
                        clustering_method->sweeping( clustering_method->buffer, x_vec );
                        cluster_temp = clustering_method->get_best();

                        likelihood_clusters.push_back( cluster_temp );
                    }

                    probeset_likelihood_clusters_temp.emplace( cube_xs[ id ], likelihood_clusters );

                    likelihood_clusters.clear();
                    x_vec.clear();
                }

                {
                    std::lock_guard< std::mutex > cluster_lock( cluster_mutex ); // TODO redundent lock
                    
                    for( auto& jobs : probeset_likelihood_clusters_temp )
                    {
                        db.probeset_likelihood_clusters[ jobs.first ] = jobs.second;
                    }
                }
            });

            cube_xs.clear();
        }

        // cluster_parallel_pool.flush_pool();
        db.thread_pool->flush_pool();
        monitor.log( "Component BRLMMp Training Tentative Clustering", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
#endif
