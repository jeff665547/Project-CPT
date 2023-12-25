#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>
#include <mutex>

namespace cpt {
namespace component {

class BRLMMpTrainingRemoveOutlier : public engine::NamedComponent
{
    using Base = engine::NamedComponent;
    std::size_t thread_num_;

    // size_t k_for_3_cluster_outlier_trimmer; 
    // size_t percentage_for_model_trimmer; 

    // size_t cluster_job_number_;
    // size_t cluster_thread_num_;

  public:

    using Base::Base;

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        // k_for_3_cluster_outlier_trimmer = p.get_optional< size_t >( "k_for_3_cluster_outlier_trimmer" ).value_or( 4 );
        // percentage_for_model_trimmer = p.get_optional< size_t >( "percentage_for_model_trimmer" ).value_or( 30 );

        // cluster_job_number_ = p.get_optional< size_t >( "cluster_job_number" ).value_or(  0 );
        // cluster_thread_num_ = p.get_optional< size_t >( "cluster_thread_num" ).value_or( 40 );

        auto& db( this->mut_data_pool() );
        thread_num_ = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );
    }

    virtual void initialize() override
    {
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        if( db.thread_pool->get_thread_num() != thread_num_ )
        {
            db.thread_pool->resize_pool( thread_num_ );
        }

        monitor.set_monitor( "Component BRLMMp Training Remove Outlier", 2 );
        monitor.log( "Component BRLMMp Training Remove Outlier", " ... " );

        size_t task_num = ( db.probeset_likelihood_clusters.size() / db.thread_pool->get_thread_num() ) +1;

        // double percentage = 1 - double( double( percentage_for_model_trimmer ) / 100.0 );

        std::mutex cluster_mutex;
        // ParaThreadPool cluster_parallel_pool( cluster_thread_num_ );

        std::vector< std::pair<                                 // pre Probeset
            size_t,                                             // id -> i
            std::vector< std::pair< double, cpt::algorithm::BRLMMpBufferType >> // likelihoods
        >> likelihood_cluster_jobs;

        for( size_t i = 0; i < db.probeset_likelihood_clusters.size(); ++i )
        {
            likelihood_cluster_jobs.emplace_back( i, std::move( db.probeset_likelihood_clusters[i] ));

            if( likelihood_cluster_jobs.size() < task_num )
            {
                if( i != db.probeset_likelihood_clusters.size()-1 )
                {
                    continue;
                }
            }

            // cluster_parallel_pool.job_post( [ &db, &cluster_mutex, likelihood_cluster_jobs, percentage ] () mutable
            // cluster_parallel_pool.job_post( [ &db, &cluster_mutex, likelihood_cluster_jobs ] () mutable
            db.thread_pool->job_post( [ &db, &cluster_mutex, likelihood_cluster_jobs ] () mutable
            {
                for( auto& jobs : likelihood_cluster_jobs )
                {
                    // db.brlmmp.outlier_trimmer( db.probeset_bic_clusters[i], db.probeset_likelihood_clusters[i], k_for_3_cluster_outlier_trimmer ); 
                    // db.brlmmp.outlier_trimmer( db.probeset_bic_clusters[ jobs.first ], jobs.second, percentage ); 
                    db.brlmmp.outlier_trimmer( db.probeset_bic_clusters[ jobs.first ], jobs.second ); 
                }

                {
                    std::lock_guard< std::mutex > cluster_lock( cluster_mutex );

                    for( auto& jobs : likelihood_cluster_jobs )
                    {
                        db.probeset_likelihood_clusters[ jobs.first ] = std::move( jobs.second );
                    }
                }
            });

            likelihood_cluster_jobs.clear();
        }

        // cluster_parallel_pool.flush_pool();
        db.thread_pool->flush();
        monitor.log( "Component BRLMMp Training Remove Outlier", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
