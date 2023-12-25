#pragma once
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <CPT/algorithm/brlmmp_buffer_type.hpp>
#include <boost/math/distributions/normal.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>
#define SWEEPING_ARGS  SampleInfo& buffer, const std::vector<double>& sample
#define GET_BEST 
#define SCORE_ESTIMATE SampleInfo& buffer, const std::vector<double>& datas  
#define CONSTRUCT_ARGS BRLMMpTbl& table
namespace cpt {
namespace algorithm {
template<class CLUSTER_RES>
struct IBRLMMpClustering : public BRLMMpClustering
{
    using ClusterRes = CLUSTER_RES;
    using SampleInfo = typename ClusterRes::second_type; // typename ClusterRes::SampleInfo;


    IBRLMMpClustering ( CONSTRUCT_ARGS )
    : BRLMMpClustering( table          )
    {}
    virtual void score_estimate_and_save( SCORE_ESTIMATE ) = 0;
    virtual ClusterRes  get_best( GET_BEST      ) = 0;
    virtual void        sweeping( SWEEPING_ARGS ) final
    {
        while ( true )
        {
            if( is_few_element( buffer.boundarys, datas->size() ))
            {
                this->skip_n++;
            }
            else
            {
                // break_point( buffer.boundarys );

                labelz_estimate( buffer, *datas );
                mean_estimate( buffer, *datas, this->skip_n );
                variance_estimate( buffer, *datas, this->skip_n );

                this->score_estimate_and_save( buffer, *datas ); 
                this->skip_n = 0;
                // debug_output( buffer );
            }

            if ( buffer.boundarys.size() == 0 )
                break;

            this->last_boundarys = buffer.boundarys;           

            if( is_border( buffer.boundarys[0], *datas ))
            {
                break;
            }

            if( is_border( buffer.boundarys[ buffer.boundarys.size() -1 ], *datas ))
            {
                boundarys_rearrange( buffer.boundarys, *datas );
                this->skip_n = 0;
            }
            else
            {
                buffer.boundarys[ buffer.boundarys.size() -1 ]++;
            }

        }
    };
};

template< class CLUSTER_RES >
using EuclideanBRLMMpBase = IBRLMMpClustering< CLUSTER_RES >;
template< class CLUSTER_RES >
struct EuclideanBRLMMp : public IBRLMMpClustering< CLUSTER_RES >
{
    using Base = EuclideanBRLMMpBase< CLUSTER_RES >;
    using SampleInfo = typename Base::SampleInfo;
    using ClusterRes = typename Base::ClusterRes;

    std::pair<double, BRLMMpBufferType> best_res {
         std::numeric_limits<double>::max(), BRLMMpBufferType()
    };
    EuclideanBRLMMp ( CONSTRUCT_ARGS )
    : Base ( table )
    // : Base ( data, n_cluster, table )
    {}

    virtual void score_estimate_and_save( SCORE_ESTIMATE ) override 
    {
        size_t idx = 0;

        std::vector< size_t > range = buffer.boundarys;
        range.emplace_back( datas.size() );

        for( size_t j = 0; j < range.size(); ++j )
        {
            for( ; idx < range[j]; ++idx )
            {
                buffer.likelihoods[j] += ( datas[ idx ] - buffer.means[ j ] ) * ( datas[ idx ] - buffer.means[ j ] );
            }

            buffer.likelihoods[j] = buffer.likelihoods[j] * buffer.thetas[ j ];

        }

        // size_t i = 0;

        // for( size_t j = 0; j < buffer.boundarys.size(); ++j )
        // {
        //     for( ; i < buffer.boundarys[j]; ++i )
        //     {
        //         buffer.likelihoods[j] += this->euclidean_update( i, buffer, datas );
        //     }
        // }

        // for( ; i < datas.size(); ++i )
        // {
        //     buffer.likelihoods[ buffer.likelihoods.size() -1 ] += this->euclidean_update( i, buffer, datas );
        // }

        auto score = this->likelihood_sum(buffer.likelihoods);
        if( score < this->best_res.first )
        {
            this->best_res.first = score;
            this->best_res.second = buffer;
        }
    };
    virtual ClusterRes get_best( GET_BEST ) override
    {
        return this->re_estimate_likelihood( this->best_res );
    };

};

template< class CLUSTER_RES >
using GaussianBRLMMpBase = IBRLMMpClustering< CLUSTER_RES >;
template< class CLUSTER_RES >
struct GaussianBRLMMp : public IBRLMMpClustering< CLUSTER_RES >
{
    using Base = GaussianBRLMMpBase< CLUSTER_RES >;
    using SampleInfo = typename Base::SampleInfo;
    using ClusterRes = typename Base::ClusterRes;

    std::pair<double, BRLMMpBufferType> best_res {
         std::numeric_limits<double>::lowest(), BRLMMpBufferType()
    };
    GaussianBRLMMp ( CONSTRUCT_ARGS )
    : Base ( table )
    // : Base ( data, n_cluster, table )
    {}

    virtual void score_estimate_and_save( SCORE_ESTIMATE ) override 
    {
        size_t idx = 0;
        double res = 0.0;

        std::vector< size_t > range = buffer.boundarys;
        range.emplace_back( datas.size() );

        std::vector< double > sqrt_variances{
            sqrt( buffer.variances[ 0 ] ),
            sqrt( buffer.variances[ 1 ] ),
            sqrt( buffer.variances[ 2 ] )
        };

        for( size_t j = 0; j < range.size(); ++j )
        {
            for( ; idx < range[j]; ++idx )
            {
                res = 0.0;

                for( size_t i = 0; i < buffer.n_cluster; ++i )
                {
                    if( isinf( buffer.variances[ i ] ) || isnan( buffer.variances[ i ] ) || buffer.variances[ i ] == 0 )
                    {
                        continue;
                    }

                    res += boost::math::pdf( boost::math::normal( buffer.means[ i ], sqrt_variances[ i ] ), datas[ idx ] ) * buffer.thetas[ i ];
                }

                buffer.likelihoods[j] += log( res );
            }
        }

        // size_t i = 0;

        // for( size_t j = 0; j < buffer.boundarys.size(); ++j )
        // {
        //     for( ; i < buffer.boundarys[j]; ++i )
        //     {
        //         buffer.likelihoods[j] += this->gaussian_update( i, buffer, datas );
        //     }
        // }

        // // cpt::dbg << __FILE__ << " : " << __LINE__ << " : " 
        // //     << buffer.likelihoods.size() << std::endl;
        // for( ; i < datas.size(); ++i )
        // {
        //     // cpt::dbg << __FILE__ << " : " << __LINE__ << " : " 
        //     //     << buffer.likelihoods[buffer.likelihoods.size() -1] << std::endl;
        //     buffer.likelihoods[ buffer.likelihoods.size() -1 ] 
        //         += this->gaussian_update( i, buffer, datas );

        //     // cpt::dbg << __FILE__ << " : " << __LINE__ << " : " 
        //     //     << buffer.likelihoods[buffer.likelihoods.size() -1] << std::endl;
        // }

        auto score = this->likelihood_sum( buffer.likelihoods );
        // cpt::dbg << __FILE__ << " : " << __LINE__ << " : " << score << std::endl;
        if( score > this->best_res.first )
        {
            this->best_res.first = score;
            this->best_res.second = buffer;
        }
    };
    virtual ClusterRes get_best( GET_BEST ) override
    {
        return best_res;
    };

};
template< class CLUSTER_RES >
using MahalanobisBRLMMpBase = IBRLMMpClustering< CLUSTER_RES >;
template< class CLUSTER_RES >
struct MahalanobisBRLMMp : public IBRLMMpClustering< CLUSTER_RES >
{
    using Base = MahalanobisBRLMMpBase< CLUSTER_RES >;
    using SampleInfo = typename Base::SampleInfo;
    using ClusterRes = typename Base::ClusterRes;

    std::pair<double, BRLMMpBufferType> best_res {
         std::numeric_limits<double>::max(), BRLMMpBufferType()
    };
    MahalanobisBRLMMp ( CONSTRUCT_ARGS )
    : Base ( table )
    {}

    virtual void score_estimate_and_save( SCORE_ESTIMATE ) override 
    {
        size_t idx = 0;

        double std_sq = 0;
        double temp = 0;

        std::vector< size_t > range = buffer.boundarys;
        range.emplace_back( datas.size() );

        for( size_t j = 0; j < range.size(); ++j )
        {
            std_sq = buffer.stds[j] * buffer.stds[j];
            temp = 2 * log( buffer.stds[j] ) * std_sq;

            for( ; idx < range[j]; ++idx )
            {
                buffer.likelihoods[j] += ( datas[idx] - buffer.means[j] ) * ( datas[idx] - buffer.means[j] ) + temp;
            }

            buffer.likelihoods[j] = buffer.likelihoods[j] * buffer.thetas[j] / std_sq;
        }

        // size_t i = 0;

        // for( size_t j = 0; j < buffer.boundarys.size(); ++j )
        // {
        //     for( ; i < buffer.boundarys[j]; ++i )
        //     {
        //         buffer.likelihoods[j] += this->mahalanobis_update( i, buffer, datas );
        //     }
        // }

        // for( ; i < datas.size(); ++i )
        // {
        //     buffer.likelihoods[ buffer.likelihoods.size() -1 ] += this->mahalanobis_update( i, buffer, datas );
        // }

        auto score = this->likelihood_sum(buffer.likelihoods);
        if( score < this->best_res.first )
        {
            this->best_res.first = score;
            this->best_res.second = buffer;
        }
    };
    virtual ClusterRes get_best( GET_BEST ) override
    {
        return this->re_estimate_likelihood( this->best_res );
    };

};

#undef SWEEPING_ARGS 
#undef GET_BEST 
#undef SCORE_ESTIMATE  
#undef CONSTRUCT_ARGS
}}
