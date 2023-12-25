#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/components/brlmmp/brlmmp_training_min_bic2.hpp>
#else
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class BRLMMpTrainingMinBic : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    // size_t cluster_job_number_;
    // size_t cluster_thread_num_;

  public:

    using Base::Base;

    virtual void config_parameters( const bpt::ptree& p ) override
    {
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

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component BRLMMp Training Min Bic", 2 );
        monitor.log( "Component BRLMMp Training Min Bic", " ... " );

        // switch( cluster_job_number_ )
        // {
        //     case 0:
        //         cluster_job_number_ = ( db.probeset_likelihood_clusters.size() / cluster_thread_num_ ) +1;
        // }
        size_t cluster_job_number_ = ( db.probeset_likelihood_clusters.size() / db.thread_pool->get_thread_num() ) +1;

        std::mutex cluster_mutex;
        // ParaThreadPool cluster_parallel_pool( cluster_thread_num_ );

        db.probeset_bic_clusters =
            std::vector<                    // pre Probeset
                std::vector<                // pre Cluster
                    std::pair<
                          double            // each Cluster bic
                        , size_t            // each Cluster K
                    >
                >
            >( db.probeset_likelihood_clusters.size() );

        std::vector< size_t > jobs;

        for( size_t i = 0; i < db.probeset_likelihood_clusters.size(); ++i )
        {
            jobs.emplace_back( i );

            if( jobs.size() < cluster_job_number_ )
            {
                if( i != db.probeset_likelihood_clusters.size()-1 )
                {
                    continue;
                }
            }

            // cluster_parallel_pool.job_post( [ &db, &cluster_mutex, jobs ] ()
            db.thread_pool->job_post( [ &db, &cluster_mutex, jobs ] ()
            {
                std::map<
                      size_t
                    , std::vector< std::pair< double, size_t >>
                > bic_clusters;

                for( auto& id : jobs )
                {
                    bic_clusters[ id ] = std::vector< std::pair< double, size_t >>();
                    auto& bic_clusters_id = bic_clusters[ id ];

                    for( size_t k = 0; k < db.probeset_likelihood_clusters[ id ].size(); ++k )
                    {
                        bic_clusters_id.emplace_back(
                              db.brlmmp.bic(
                                  db.probeset_likelihood_clusters[ id ][ k ].first
                                , k+1
                                , db.probeset_likelihood_clusters[ id ][ k ].second.data.size()
                              )
                            , k
                        );
                    }

                    std::sort( bic_clusters_id.begin(), bic_clusters_id.end(),
                        [ &bic_clusters_id ]( std::pair< double, size_t >& a, std::pair< double, size_t >& b )
                        {
                            return a.first < b.first;
                        }
                    );
                }

                {
                    std::lock_guard< std::mutex > cluster_lock( cluster_mutex );

                    for( auto& jobs : bic_clusters )
                    {
                        db.probeset_bic_clusters[ jobs.first ] = std::move( jobs.second );
                    }
                }
            });

            jobs.clear();
        }

        // cluster_parallel_pool.flush_pool();
        db.thread_pool->flush_pool();
        monitor.log( "Component BRLMMp Training Min Bic", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
#endif
