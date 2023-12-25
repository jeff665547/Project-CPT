#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
#include <mutex>
#include <functional>
#include <CPT/utility/gprofiler.hpp>
#include <CPT/utility.hpp>
#include <CPT/algorithm/brlmmp_clustering.hpp>
#include <CPT/engine/data_pool/component_object_manager.hpp>
namespace cpt {
namespace component {
namespace cu = cpt::utility;
namespace ca = cpt::algorithm;
namespace com_ = cpt::engine::data_pool::component_object_manager;
namespace cf_ = cpt::format;
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
    std::size_t thread_num_;

    /* input */
    com_::ICPObjPtr< cf_::Cube<double> > probeset_cube_;

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
        auto json = cf_::make_json( p );
        k_cluster_   = p.get_optional< size_t      >( "k_cluster.content"   ).value_or( 3 );
        brlmmp_type_ = p.get_optional< std::string >( "brlmmp_type.content" ).value_or( "Euclidean" );

        auto& db( this->mut_data_pool() );
        thread_num_ = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );
        probeset_cube_ = com_::require_w<cf_::Cube<double>>(
            json, "probeset_cube", com_::make_ref_parameter("probeset_cube")
        );
    }

    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );
        if( db.thread_pool->get_thread_num() != thread_num_ )
        {
            db.thread_pool->resize_pool( thread_num_ );
        }
        probeset_cube_ -> initialize();
    }

    virtual void NOINLINE start() override
    {
        cu::GProfiler profiler("BRLMMpTrainingTentativeClustering_start_2");
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();
        auto& probeset_cube = probeset_cube_ -> get();

        size_t cube_x = probeset_cube.n_rows; /* probeset */
        size_t cube_y = probeset_cube.n_cols; /* sample */

        monitor.set_monitor( "Component BRLMMp Training Tentative Clustering", 2 );
        monitor.log( "Component BRLMMp Training Tentative Clustering", " ... " );
        size_t task_num = ( cube_x / db.thread_pool->get_thread_num() ) +1;

        std::mutex cluster_mutex;

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

            if( cube_xs.size() < task_num )
            {
                if( i != cube_x-1 )
                {
                    continue;
                }
            }

            // cluster_parallel_pool.job_post( [ &clustering_method_pre, cube_xs, &db, &cube_y, &cluster_mutex, &probeset_likelihood_clusters, this ] ()
            db.thread_pool->job_post( [ 
                &clustering_method_pre
                , cube_xs
                , &db
                , &cube_y
                , &cluster_mutex
                , &probeset_likelihood_clusters
                , &probeset_cube
                , this 
            ] ()
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
                        x_vec.emplace_back( probeset_cube( cube_xs[ id ], j, 0 ));
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
        db.thread_pool->flush();
        monitor.log( "Component BRLMMp Training Tentative Clustering", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
#else
#endif
