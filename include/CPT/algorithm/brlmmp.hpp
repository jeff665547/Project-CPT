#pragma once
#ifdef JOYE_VER

#include <CPT/algorithm/brlmmp_joye.hpp>

#else

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <boost/math/distributions/normal.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CPT/utility.hpp>
#include <CPT/algorithm/brlmmp_buffer_type.hpp>
#include <CPT/utility.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace algorithm {

struct BRLMMpTbl
{
    std::vector< double > last_means;
    std::vector< double > last_variances;
    std::vector< size_t > last_boundarys;
    size_t skip_n;

    BRLMMpBufferType buffer;
    std::vector< std::pair< double, BRLMMpBufferType >> likelihoods;

    BRLMMpTbl ( const std::vector< double >& data, const size_t& n_cluster )
    : last_means        ( n_cluster, 0 )
    , last_variances    ( n_cluster, 0 )
    , last_boundarys    ( n_cluster, 0 )
    , skip_n            ( 0 )
    , buffer            ( data, n_cluster )
    , likelihoods       ()
    {}
    BRLMMpTbl() = default;

    void reset ( const std::vector< double >& data, const size_t& n_cluster )
    {
        cpt::reset_vector ( last_means,      n_cluster, 0.0             );
        cpt::reset_vector ( last_variances,  n_cluster, 0.0             );
        cpt::reset_vector ( last_boundarys,  n_cluster, std::size_t(0)  );
        skip_n = 0;
        buffer.reset ( data, n_cluster );
    }
};

class BRLMMpClustering
{
  public:

    const std::vector< double >* datas;
    std::vector< double >      & last_means;
    std::vector< double >      & last_variances;
    std::vector< size_t >      & last_boundarys;
    size_t                     & skip_n;

    BRLMMpBufferType           & buffer;
    std::vector< 
        std::pair< 
              double
            , BRLMMpBufferType 
        >
    >                          & likelihoods;
    BRLMMpTbl                  & table;
    BRLMMpClustering( const std::vector< double >& data, const size_t& n_cluster, BRLMMpTbl& _table ) 
        : datas             ( &data )
        , last_means        ( _table.last_means      )
        , last_variances    ( _table.last_variances  )
        , last_boundarys    ( _table.last_boundarys  )
        , skip_n            ( _table.skip_n          )
        , buffer            ( _table.buffer          )
        , likelihoods       ( _table.likelihoods     )
        , table             ( _table                 )
    {
    }
    BRLMMpClustering( BRLMMpTbl& _table ) 
        : datas             ( nullptr )
        , last_means        ( _table.last_means      )
        , last_variances    ( _table.last_variances  )
        , last_boundarys    ( _table.last_boundarys  )
        , skip_n            ( _table.skip_n          )
        , buffer            ( _table.buffer          )
        , likelihoods       ( _table.likelihoods     )
        , table             ( _table                )
    {
    }
    void reset( const std::vector< double >& data, const size_t& n_cluster )
    {
        datas = &data;
        table.reset( data, n_cluster );
    }

    bool is_few_element( const std::vector< size_t >& boundarys, const size_t& datas_size )
    {
        for( size_t i = 0; i < boundarys.size(); ++i )
        {
            if( ( i == 0 && boundarys[i] < 2 )||
                ( i != 0 && boundarys[i] - boundarys[i-1] < 2 )||
                ( i == boundarys.size()-1 && datas_size - boundarys[i] < 2 )
              )
            {
                return true;
            }
        }

        return false;

    }

    bool is_border( const double& boundary, const std::vector< double >& datas )
    {
        if( boundary >= ( datas.size() ))
        {
            return true;
        }
        return false;
    }

    double likelihood_sum( std::vector< double >& likeli )
    {
        double res( 0.0 );

        for( auto& likelihood : likeli )
        {
            // cpt::dbg << __FILE__ << " : " << __LINE__ 
            //     << ":" << likelihood << std::endl;
            res += likelihood;
            likelihood = 0;
            // cpt::dbg << __FILE__ << " : " << __LINE__ 
            //     << ":" << res << std::endl;
        }
        // cpt::dbg << __FILE__ << " : " << __LINE__ 
        //     << ":" <<  res << std::endl;

        return res;
    }

    void labelz_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas )
    {
        switch( buffer.boundarys.size() )
        {
            case 0:
                buffer.thetas[0] = 1;
                break;

            default:
                buffer.thetas[0] = double( buffer.boundarys[0] ) / double( datas.size() );

                for( size_t i = 1; i < buffer.boundarys.size(); ++i )
                {
                    buffer.thetas[i] = double( buffer.boundarys[i] - buffer.boundarys[ i-1 ] ) / double( datas.size() );
                }

                buffer.thetas[ buffer.boundarys.size() ] = double( datas.size() - buffer.boundarys[ buffer.boundarys.size() -1 ] ) / double( datas.size() );
        }
    }

    void incremental_mean_estimate( const size_t& idx, const double& this_n, const double& this_num, std::vector< double >& means )
    {
        means[ idx ] = this->last_means[ idx ] + ( this_n - this->last_means[ idx ] ) / this_num;
    }

    void decremental_mean_estimate( const size_t& idx, const double& last_n, const double& last_num, std::vector< double >& means )
    {
        means[ idx ] = ( 1 / ( last_num -1 )) * (( last_num * this->last_means[ idx ] ) - last_n );
    }

    void boundary_mean_estimate( const size_t& idx, const size_t& start, const size_t& end, std::vector< double >& means, const std::vector< double >& datas )
    {
        double sum( 0.0 );

        for( size_t i = start; i < end; ++i )
        {
            sum += datas[ i ];
        }

        means[ idx ] = sum / ( end - start );
    }

    void first_cluster_mean_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        if( buffer.boundarys[0] == 0 )
        {
            buffer.means[0] = 0;
            return;
        }
        else {}

        if( skipn != 0 )
        {
            boundary_mean_estimate(
                    0,
                    0,
                    buffer.boundarys[ 0 ],
                    buffer.means,
                    datas
                    );
        }
        else
        {
            if( buffer.boundarys[0] != this->last_boundarys[0] )
            {
                incremental_mean_estimate( 0, datas[ buffer.boundarys[0] -1 ], buffer.boundarys[0], buffer.means );
            }
            else {}
        }

        // switch( buffer.boundarys[0] )
        // {
        //     case 0:
        //         buffer.means[0] = 0;
        //         break;

        //     default:
        //         if( buffer.boundarys[0] != this->last_boundarys[0] )
        //         {
        //             incremental_mean_estimate( 0, datas[ buffer.boundarys[0] -1 ], buffer.boundarys[0], buffer.means );
        //         }
        //         else {}
        // }
    }

    void cluster_mean_estimate( const size_t& i, BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        if( buffer.boundarys[i] == datas.size() ||
            buffer.boundarys[i] < this->last_boundarys[i] ||
            buffer.boundarys[i] == buffer.boundarys[ i-1 ]
        )
        {
            buffer.means[i] = 0;
            return;
        }
        else {}

        if( buffer.means[i] != 0 && buffer.boundarys[i] == this->last_boundarys[i] )
        {
            return;
        }
        else {}

        if( skipn != 0 )
        {
            for( size_t sk = skipn -1; sk > 0; sk-- )
            {
                incremental_mean_estimate( i, datas[ buffer.boundarys[i] -1 -sk ], buffer.boundarys[i] - buffer.boundarys[ i-1 ] -sk, buffer.means );
                this->last_means[i] = buffer.means[i];
            }
        }

        incremental_mean_estimate( i, datas[ buffer.boundarys[i] -1 ], buffer.boundarys[i] - buffer.boundarys[ i-1 ], buffer.means );

        // if( buffer.boundarys[i] == datas.size() ||
        //     buffer.boundarys[i] < this->last_boundarys[i] ||
        //     buffer.boundarys[i] == buffer.boundarys[ i-1 ]
        // )
        // {
        //     buffer.means[i] = 0;
        //     return;
        // }
        // else {}

        // if( buffer.boundarys[i] != this->last_boundarys[i] )
        // {
        //     incremental_mean_estimate( i, datas[ buffer.boundarys[i] -1 ], buffer.boundarys[i] - buffer.boundarys[ i-1 ], buffer.means );
        // }
        // else {}
    }

    void last_cluster_mean_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        if( buffer.boundarys[ buffer.boundarys.size() -1 ] == datas.size() )
        {
            buffer.means[ buffer.boundarys.size() ] = 0;
            return;
        }
        else {}

        if( skipn != 0 || buffer.boundarys[ buffer.boundarys.size() -1 ] < this->last_boundarys[ this->last_boundarys.size() -1 ] )
        {
            boundary_mean_estimate(
                    buffer.boundarys.size(),
                    buffer.boundarys[ buffer.boundarys.size() -1 ],
                    datas.size(),
                    buffer.means,
                    datas
                    );
        }
        else
        {
            decremental_mean_estimate(
                    buffer.boundarys.size(),
                    datas[ buffer.boundarys[ buffer.boundarys.size() -1 ] -1 ],
                    datas.size() - buffer.boundarys[ buffer.boundarys.size() -1 ] +1,
                    buffer.means
                    );
        }

        // if( buffer.boundarys[ buffer.boundarys.size() -1 ] == datas.size() )
        // {
        //     buffer.means[ buffer.boundarys.size() ] = 0;
        //     return;
        // }
        // else {}

        // if( buffer.boundarys[ buffer.boundarys.size() -1 ] > this->last_boundarys[ this->last_boundarys.size() -1 ] )
        // {
        //     decremental_mean_estimate(
        //             buffer.boundarys.size(),
        //             datas[ buffer.boundarys[ buffer.boundarys.size() -1 ] -1 ],
        //             datas.size() - buffer.boundarys[ buffer.boundarys.size() -1 ] +1,
        //             buffer.means
        //             );
        // }
        // else
        // {
        //     boundary_mean_estimate(
        //             buffer.boundarys.size(),
        //             buffer.boundarys[ buffer.boundarys.size() -1 ],
        //             datas.size(),
        //             buffer.means,
        //             datas
        //             );
        // }
    }

    void mean_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        switch( buffer.boundarys.size() )
        {
            case 0:
                boundary_mean_estimate( 0, 0, datas.size(), buffer.means, datas );
                break;

            default:
                this->last_means = buffer.means;

                first_cluster_mean_estimate( buffer, datas, skipn );

                for( size_t i = 1; i < buffer.boundarys.size(); ++i )
                {
                    cluster_mean_estimate( i, buffer, datas, skipn );
                }

                last_cluster_mean_estimate( buffer, datas, skipn );
        }
    }

    void incremental_variance_estimate( const size_t& idx, const double& this_n, const double& this_num, std::vector< double >& variances )
    {
        variances[ idx ] = ( this_num -2 ) / ( this_num -1 ) * this->last_variances[ idx ] + ( 1 / this_num ) * std::pow( this->last_means[ idx ] - this_n, 2 );
    }

    void decremental_variance_estimate( const size_t& idx, const double& last_n, const double& last_num, std::vector< double >& variances, std::vector< double >& means )
    {
        variances[ idx ] = ( last_num - 1 ) / ( last_num -2 ) * ( this->last_variances[ idx ] - ( 1 /  last_num ) * std::pow( means[ idx ] - last_n, 2 ));
    }

    void boundary_variance_estimate( const size_t& idx, const size_t& start, const size_t& end, std::vector< double >& variances, const std::vector< double >& datas )
    {
        double sum( 0.0 );

        for( size_t i = start; i < end; ++i )
        {
            sum += datas[ i ];
        }

        double mean( sum / ( end - start ));
        sum = 0;

        for( size_t i = start; i < end; ++i )
        {
            sum += std::pow( datas[ i ] - mean, 2);
        }

        variances[ idx ] = sum / (( end - start ) - 1 );
    }

    void first_cluster_variance_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        if( skipn != 0 || buffer.boundarys[0] == 2 )
        {
            boundary_variance_estimate( 0, 0, buffer.boundarys[0], buffer.variances, datas );
            return;
        }
        else {}

        if( buffer.boundarys[0] > this->last_boundarys[0] )
        {
            incremental_variance_estimate( 0, datas[ buffer.boundarys[0] -1 ], buffer.boundarys[0], buffer.variances );
        }
        else {}

        // switch( buffer.boundarys[0] )
        // {
        //     case 0:
        //         buffer.variances[0] = 0;
        //         break;

        //     case 1:
        //         buffer.variances[0] = 0;
        //         break;

        //     case 2:
        //         boundary_variance_estimate( 0, 0, buffer.boundarys[0], buffer.variances, datas );
        //         break;

        //     default:
        //         if( buffer.boundarys[0] > this->last_boundarys[0] )
        //         {
        //             incremental_variance_estimate( 0, datas[ buffer.boundarys[0] -1 ], buffer.boundarys[0], buffer.variances );
        //         }
        //         else {}
        // }
    }

    void cluster_variance_estimate( const size_t& i, BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        if( buffer.boundarys[i] - buffer.boundarys[ i-1 ] == 2 )
        {
            boundary_variance_estimate( i, buffer.boundarys[ i-1 ], buffer.boundarys[i], buffer.variances, datas );
            return;
        }
        else {}

        if( buffer.means[0] != 0 && buffer.boundarys[i] == this->last_boundarys[i] )
        {
            return;
        }
        else {}

        if( skipn != 0 )
        {
            for( size_t sk = skipn -1; sk > 0; sk-- )
            {
                incremental_variance_estimate( i, datas[ buffer.boundarys[i] -1 -sk ], buffer.boundarys[i] - buffer.boundarys[ i-1 ] -sk, buffer.variances );
                this->last_variances[i] = buffer.variances[i];
            }
        }

        incremental_variance_estimate( i, datas[ buffer.boundarys[i] -1 ], buffer.boundarys[i] - buffer.boundarys[ i-1 ], buffer.variances );

        // if( buffer.boundarys[i] == datas.size() ||
        //     buffer.boundarys[i] < this->last_boundarys[i]
        // )
        // {
        //     buffer.variances[i] = 0;
        //     return;
        // }
        // else {}

        // switch( buffer.boundarys[i] - buffer.boundarys[ i-1 ] )
        // {
        //     case 0:
        //         buffer.variances[i] = 0;
        //         break;

        //     case 1:
        //         buffer.variances[i] = 0;
        //         break;

        //     case 2:
        //         boundary_variance_estimate( i, buffer.boundarys[ i-1 ], buffer.boundarys[i], buffer.variances, datas );
        //         break;

        //     default:
        //         if( buffer.boundarys[i] > this->last_boundarys[i] )
        //         {
        //             incremental_variance_estimate( i, datas[ buffer.boundarys[i] -1 ], buffer.boundarys[i] - buffer.boundarys[ i-1 ], buffer.variances );
        //         }
        //         else {}
        // }
    }

    void last_cluster_variance_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        if( skipn != 0 || buffer.boundarys[ buffer.boundarys.size() -1 ] < this->last_boundarys[ this->last_boundarys.size() -1 ] )
        {
            boundary_variance_estimate(
                    buffer.boundarys.size(),
                    buffer.boundarys[ buffer.boundarys.size() -1 ],
                    datas.size(),
                    buffer.variances,
                    datas
                    );
        }
        else
        {
            boundary_variance_estimate(
                    buffer.boundarys.size(),
                    buffer.boundarys[ buffer.boundarys.size() -1 ],
                    datas.size(),
                    buffer.variances,
                    datas
                    );
        }

        // switch( datas.size() - buffer.boundarys[ buffer.boundarys.size() -1 ] )
        // {
        //     case 0:
        //         buffer.variances[ buffer.boundarys.size() ] = 0;
        //         break;

        //     case 1:
        //         buffer.variances[ buffer.boundarys.size() ] = 0;
        //         break;

        //     default:
        //         if( buffer.boundarys[ buffer.boundarys.size() -1 ] > this->last_boundarys[ this->last_boundarys.size() -1 ] )
        //         {
        //             decremental_variance_estimate(
        //                     buffer.boundarys.size(),
        //                     datas[ buffer.boundarys[ buffer.boundarys.size() -1 ] -1 ],
        //                     datas.size() - buffer.boundarys[ buffer.boundarys.size() -1 ] +1,
        //                     buffer.variances,
        //                     buffer.means
        //                     );
        //         }
        //         else
        //         {
        //             boundary_variance_estimate(
        //                     buffer.boundarys.size(),
        //                     buffer.boundarys[ buffer.boundarys.size() -1 ],
        //                     datas.size(),
        //                     buffer.variances,
        //                     datas
        //                     );
        //         }
        // }
    }

    void std_estimate( BRLMMpBufferType& buffer )
    {
        for( size_t i = 0; i < buffer.n_cluster; ++i )
        {
            buffer.stds[i] = sqrt( buffer.variances[i] );
        }
    }

    void variance_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, const size_t& skipn )
    {
        switch( buffer.boundarys.size() )
        {
            case 0:
                boundary_variance_estimate( 0, 0, datas.size(), buffer.variances, datas );
                break;

            default:
                this->last_variances = buffer.variances;

                first_cluster_variance_estimate( buffer, datas, skipn );

                for( size_t i = 1; i < buffer.boundarys.size(); ++i )
                {
                    cluster_variance_estimate( i, buffer, datas, skipn );
                }

                last_cluster_variance_estimate( buffer, datas, skipn );
        }

        std_estimate( buffer );
    }

    void likelihood_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, std::vector< std::pair< double, BRLMMpBufferType >>& likelis )
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

        likelis.emplace_back( likelihood_sum( buffer.likelihoods ), buffer );
    }

    // double gaussian_update( const size_t& idx, BRLMMpBufferType& buffer, const std::vector< double >& datas )
    // {
    //     double res( 0.0 );
    //     double p( 0.0 );

    //     for( size_t i = 0; i < buffer.n_cluster; ++i )
    //     {
    //         if( isinf( buffer.variances[ i ] ) || isnan( buffer.variances[ i ] ) || buffer.variances[ i ] == 0 )
    //         {
    //             continue;
    //         }

    //         p = boost::math::pdf(
    //             boost::math::normal(
    //                 buffer.means[ i ],
    //                 sqrt( buffer.variances[ i ] )
    //             ),
    //             datas[ idx ]
    //         );

    //         res += ( p * buffer.thetas[ i ] );
    //     }

    //     // cpt::dbg << __FILE__ << ":" << __LINE__ << ":" <<  res << std::endl;
    //     res = log( res );
    //     return res;
    // }

    // void likelihood_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, std::vector< std::pair< double, BRLMMpBufferType >>& likelis )
    // {
    //     size_t i = 0;

    //     for( size_t j = 0; j < buffer.boundarys.size(); ++j )
    //     {
    //         for( ; i < buffer.boundarys[j]; ++i )
    //         {
    //             buffer.likelihoods[j] += gaussian_update( i, buffer, datas );
    //         }
    //     }

    //     for( ; i < datas.size(); ++i )
    //     {
    //         buffer.likelihoods[ buffer.likelihoods.size() -1 ] += gaussian_update( i, buffer, datas );
    //     }

    //     likelis.emplace_back( likelihood_sum( buffer.likelihoods ), buffer );
    // }

    std::pair< double, BRLMMpBufferType > re_estimate_likelihood( std::pair< double, BRLMMpBufferType >& likeli )
    {
        size_t idx = 0;
        double res = 0.0;

        std::vector< size_t > range = likeli.second.boundarys;
        range.emplace_back( likeli.second.data.size() );

        std::vector< double > sqrt_variances{
            sqrt( likeli.second.variances[ 0 ] ),
            sqrt( likeli.second.variances[ 1 ] ),
            sqrt( likeli.second.variances[ 2 ] )
        };

        for( size_t j = 0; j < range.size(); ++j )
        {
            for( ; idx < range[j]; ++idx )
            {
                res = 0.0;

                for( size_t i = 0; i < likeli.second.n_cluster; ++i )
                {
                    if( isinf( likeli.second.variances[ i ] ) || isnan( likeli.second.variances[ i ] ) || likeli.second.variances[ i ] == 0 )
                    {
                        continue;
                    }

                    res += boost::math::pdf( boost::math::normal( likeli.second.means[ i ], sqrt_variances[ i ] ), likeli.second.data[ idx ] ) * likeli.second.thetas[ i ];
                }

                likeli.second.likelihoods[j] += log( res );
            }
        }

        likeli.first = likelihood_sum( likeli.second.likelihoods );
        return likeli;
    }

    // std::pair< double, BRLMMpBufferType > re_estimate_likelihood( std::pair< double, BRLMMpBufferType >& likeli )
    // {
    //     size_t i = 0;

    //     for( size_t j = 0; j < likeli.second.boundarys.size(); ++j )
    //     {
    //         for( ; i < likeli.second.boundarys[j]; ++i )
    //         {
    //             likeli.second.likelihoods[j] += gaussian_update( i, likeli.second, likeli.second.data );
    //         }
    //     }

    //     for( ; i < likeli.second.data.size(); ++i )
    //     {
    //         likeli.second.likelihoods[ likeli.second.likelihoods.size() -1 ] += gaussian_update( i, likeli.second, likeli.second.data );
    //     }

    //     likeli.first = likelihood_sum( likeli.second.likelihoods );

    //     return likeli;
    // }

    size_t get_cluster_k( const size_t& idx, const std::vector< size_t >& boundarys )
    {
        if( boundarys.empty() || idx < boundarys[0] )
        {
            return 0;
        }

        if( idx >= boundarys[ boundarys.size() -1 ] )
        {
            return boundarys.size();
        }

        size_t k( 0 );
        size_t last_boundary( 0 );

        for( size_t i = 1; i < boundarys.size(); ++i )
        {
            if( idx+1 > last_boundary && idx+1 <= boundarys[ i ] )
            {
                k = i;
                break;
            }

            last_boundary = boundarys[ i ];
        }

        return k;
    }

    /**
     *  buffer.likelihoods[j] += ((( datas[idx] - buffer.means[j] ) / buffer.stds[j] ) * (( datas[idx] - buffer.means[j] ) / buffer.stds[j] ) + ( 2 * log( buffer.stds[j] ))) * buffer.thetas[j];
     *  buffer.likelihoods[j] += (( datas[idx] - buffer.means[j] ) * ( datas[idx] - buffer.means[j] ) / ( buffer.stds[j] * buffer.stds[j] )) + ( 2 * log( buffer.stds[j] ));
     *  buffer.likelihoods[j] += (( datas[idx] - buffer.means[j] ) * ( datas[idx] - buffer.means[j] ) / ( buffer.stds[j] * buffer.stds[j] )) + ( 2 * log( buffer.stds[j] ));
     *  buffer.likelihoods[j] += (( datas[idx] - buffer.means[j] ) * ( datas[idx] - buffer.means[j] ) + ( 2 * log( buffer.stds[j] ) * buffer.stds[j] * buffer.stds[j] )) / ( buffer.stds[j] * buffer.stds[j] );
    **/
    void total_mahalanobis_distance_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, std::vector< std::pair< double, BRLMMpBufferType >>& likelis )
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

        likelis.emplace_back( likelihood_sum( buffer.likelihoods ), buffer );
    }

    // double mahalanobis_update( const size_t& idx, BRLMMpBufferType& buffer, const std::vector< double >& datas )
    // {
    //     size_t k = get_cluster_k( idx, buffer.boundarys );

    //     double p(( datas[ idx ] - buffer.means[ k ] ) / ( buffer.stds[ k ] ));
    //     return(( p * p + ( log( buffer.stds[ k ] ) * 2 )) * buffer.thetas[ k ] );
    // }

    // void total_mahalanobis_distance_estimate( BRLMMpBufferType& buffer, const std::vector< double >& datas, std::vector< std::pair< double, BRLMMpBufferType >>& likelis )
    // {
    //     size_t i = 0;

    //     for( size_t j = 0; j < buffer.boundarys.size(); ++j )
    //     {
    //         for( ; i < buffer.boundarys[j]; ++i )
    //         {
    //             buffer.likelihoods[j] += mahalanobis_update( i, buffer, datas );
    //         }
    //     }

    //     for( ; i < datas.size(); ++i )
    //     {
    //         buffer.likelihoods[ buffer.likelihoods.size() -1 ] += mahalanobis_update( i, buffer, datas );
    //     }

    //     likelis.emplace_back( likelihood_sum( buffer.likelihoods ), buffer );
    // }
    //
    
    /**
     *  buffer.likelihoods[j] += ( datas[ idx ] - buffer.means[ j ] ) * ( datas[ idx ] - buffer.means[ j ] ) * buffer.thetas[ j ];
    **/
    void NOINLINE total_euclidean_distance_estimate( 
          BRLMMpBufferType& buffer
        , const std::vector< double >& datas
        , std::vector< std::pair< double, BRLMMpBufferType >>& likelis 
    )
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

        likelis.emplace_back( likelihood_sum( buffer.likelihoods ), buffer );
    }

    // double euclidean_update( const size_t& idx, BRLMMpBufferType& buffer, const std::vector< double >& datas )
    // {
    //     size_t k = get_cluster_k( idx, buffer.boundarys );

    //     double p( datas[ idx ] - buffer.means[ k ] );
    //     return( p * p * buffer.thetas[ k ] );
    // }

    // // double best_score { std::numeric_limits<double>::max() };
    // // BRLMMpBufferType best_buf;
    // void NOINLINE total_euclidean_distance_estimate( 
    //       BRLMMpBufferType& buffer
    //     , const std::vector< double >& datas
    //     , std::vector< std::pair< double, BRLMMpBufferType >>& likelis 
    // )
    // {
    //     size_t i = 0;

    //     for( size_t j = 0; j < buffer.boundarys.size(); ++j )
    //     {
    //         for( ; i < buffer.boundarys[j]; ++i )
    //         {
    //             buffer.likelihoods[j] += euclidean_update( i, buffer, datas );
    //         }
    //     }

    //     for( ; i < datas.size(); ++i )
    //     {
    //         buffer.likelihoods[ buffer.likelihoods.size() -1 ] += euclidean_update( i, buffer, datas );
    //     }
    //     likelis.emplace_back( likelihood_sum( buffer.likelihoods ), buffer );
    // }

    size_t get_max_likelihood( const std::vector< std::pair< double, BRLMMpBufferType >>& likelis )
    {
        size_t max_index( 0 );
        double value( likelis[ 0 ].first );

        for( size_t i = 0; i < likelis.size(); ++i )
        {
            const auto& get_value = likelis[ i ].first;

            if( value < get_value )
            {
                value = get_value;
                max_index = i;
            }
        }

        return max_index;
    }

    size_t get_min_distance( const std::vector< std::pair< double, BRLMMpBufferType >>& likelis )
    {
        size_t min_index( 0 );
        double value( likelis[ 0 ].first );

        for( size_t i = 0; i < likelis.size(); ++i )
        {
            const auto& get_value = likelis[ i ].first;

            if( value > get_value )
            {
                value = get_value;
                min_index = i;
            }
        }

        return min_index;
    }


    void boundarys_rearrange( std::vector< size_t >& boundarys, const std::vector< double >& datas )
    {
        int i = boundarys.size() -2;

        for( ; i >= 0; --i )
        {
            if( !is_border( boundarys[i], datas ))
            {
                boundarys[i]++;
                break;
            }
        }

        const auto& start_point = boundarys[i];

        for( ; i < boundarys.size(); ++i )
        {
            boundarys[i] = start_point;
        }
    }

    bool sweeping(
              BRLMMpBufferType& buffer
            , const std::vector< double >& datas
            , std::vector< std::pair< double, BRLMMpBufferType >>& likelis
            , const std::string& brlmmp_type
    )
    {
        if( is_few_element( buffer.boundarys, datas.size() ))
        {
            this->skip_n++;
        }
        else
        {
            // break_point( buffer.boundarys );

            labelz_estimate( buffer, datas );
            mean_estimate( buffer, datas, this->skip_n );
            variance_estimate( buffer, datas, this->skip_n );

            if( brlmmp_type == "Gaussian" )
            {
                likelihood_estimate( buffer, datas, likelis );      //Gaussian
            }
            else if( brlmmp_type == "Mahalanobis" )
            {
                total_mahalanobis_distance_estimate( buffer, datas, likelis );  //Mahalanobis
            }
            else if( brlmmp_type == "Euclidean" )
            {
                total_euclidean_distance_estimate( buffer, datas, likelis );    //Euclidean
            }
            else
            {
                throw std::runtime_error( "Error for brlmmp_type : " + brlmmp_type + " in brlmmp::sweeping()" );
            }

            this->skip_n = 0;
            // debug_output( buffer );
        }

        switch( buffer.boundarys.size() )
        {
            case 0:
                return false;
        }

        this->last_boundarys = buffer.boundarys;           

        if( is_border( buffer.boundarys[0], datas ))
        {
            return false;
        }

        if( is_border( buffer.boundarys[ buffer.boundarys.size() -1 ], datas ))
        {
            boundarys_rearrange( buffer.boundarys, datas );
            this->skip_n = 0;
        }
        else
        {
            buffer.boundarys[ buffer.boundarys.size() -1 ]++;
        }

        return true;
    }
    // std::vector< std::pair< double, BRLMMpBufferType >> likelisx;
    // void sweeping2(
    //           BRLMMpBufferType& buffer
    //         , const std::vector< double >& datas
    //         , const std::string& brlmmp_type
    // )
    // {
    // }

    std::pair< double, BRLMMpBufferType > run( const std::string& brlmmp_type )
    {
        while( true )
        {
            if( !sweeping( this->buffer, *(this->datas), this->likelihoods, brlmmp_type ))
            {
                break;
            }
        }

        if( brlmmp_type == "Gaussian" )
        {
            return this->likelihoods[ get_max_likelihood( this->likelihoods ) ];
        }
        else if( brlmmp_type == "Mahalanobis" || brlmmp_type == "Euclidean" )
        {
            return re_estimate_likelihood( this->likelihoods[ get_min_distance( this->likelihoods )]); //Distance
        }
        else
        {
            throw std::runtime_error( "Error for brlmmp_type : " + brlmmp_type + " in brlmmp::run()" );
        }
    }

    std::pair< double, BRLMMpBufferType > run(
              BRLMMpBufferType& buffer
            , const std::vector< double >& datas
            , const std::string& brlmmp_type )
    {
        while( true )
        {
            if( !sweeping( buffer, datas, this->likelihoods, brlmmp_type ))
            {
                break;
            }
        }

        if( brlmmp_type == "Gaussian" )
        {
            return this->likelihoods[ get_max_likelihood( this->likelihoods ) ];
        }
        else if( brlmmp_type == "Mahalanobis" || brlmmp_type == "Euclidean" )
        {
            return re_estimate_likelihood( this->likelihoods[ get_min_distance( this->likelihoods )]); //Distance
        }
        else
        {
            throw std::runtime_error( "Error for brlmmp_type : " + brlmmp_type + " in brlmmp::run()" );
        }
    }

    std::pair< double, BRLMMpBufferType > run(
          BRLMMpBufferType& buffer
        , const std::vector< double >& datas
        , std::vector< std::pair< double, BRLMMpBufferType >>& likelis
        , const std::string& brlmmp_type )
    {
        while( true )
        {
            if( !sweeping( buffer, datas, likelis, brlmmp_type ))
            {
                break;
            }
        }

        if( brlmmp_type == "Gaussian" )
        {
            return this->likelihoods[ get_max_likelihood( this->likelihoods ) ];
        }
        else if( brlmmp_type == "Mahalanobis" || brlmmp_type == "Euclidean" )
        {
            return re_estimate_likelihood( this->likelihoods[ get_min_distance( this->likelihoods )]); //Distance
        }
        else
        {
            throw std::runtime_error( "Error for brlmmp_type : " + brlmmp_type + " in brlmmp::run()" );
        }
    }

    void break_point( const std::vector< size_t >& boundarys )
    {
        bool bp = false;

        if( boundarys == std::vector< size_t >{ 0, 1 } )
            bp = true;
    }

    void debug_output( const BRLMMpBufferType& buffer )
    {
        std::cerr << "boundary:";

        for( auto& boundary : buffer.boundarys )
        {
            std::cerr << " " << boundary;
        }

        std::cerr
            << "\tlikelihood:"
            << likelihoods[ likelihoods.size()-1 ].first
            ;

        std::cerr << "\tlabelz:";

        for( auto& labelz : buffer.thetas )
        {
            std::cerr << " " << labelz;
        }

        std::cerr << "\tmean:";

        for( auto& mean : buffer.means )
        {
            std::cerr << " " << mean;
        }

        std::cerr << "\tvariance:";

        for( auto& variance : buffer.variances )
        {
            std::cerr << " " << variance;
        }

        std::cerr << "\n\n";
    }
};

typedef std::vector< std::pair< std::vector< double >, std::vector< double >>> BRLMMpTrainingType;

class BRLMMp
{
    size_t k_cluster_;
    std::string brlmmp_type_;

    std::vector< double > means_of_mean_;
    std::vector< double > means_of_std_;

    std::vector< std::vector< double >> variances_of_mean_;
    std::vector< std::vector< double >> variances_of_std_;

    std::map< size_t, std::vector< size_t >> bic_buffer_idxs_;

    void check_cluster_k( const size_t& k)
    {
        switch( k )
        {
            case 3:
                break;

            default:
                throw std::runtime_error( "Cluster K must be 3 in BRLMMp\n" );
        }
    }

    void set_bic_buffer_idxs( std::map< size_t, std::vector< size_t >>& bic_buffer_idxs )
    {
        for( size_t i = 1; i <= k_cluster_; ++i )
        {
            bic_buffer_idxs.emplace( i, std::vector< size_t >() );
        }
    }

  public:

    void print_of_test()
    {
        std::cout << "\n\nmeanOFmean:";
        for( auto& mean : means_of_mean_ )
            std::cerr << "\t" << mean;

        std::cout << "\n\nmeanOFstd:";
        for( auto& std : means_of_std_ )
            std::cerr << "\t" << std;

        std::cout << "\n\nvarianceOFmean:\n";
        for( size_t a = 0; a < variances_of_mean_.size(); ++a )
        {
            for( size_t b = 0; b < variances_of_mean_.size(); ++b )
                std::cerr << "\t" << variances_of_mean_[a][b];
            std::cerr << "\n";
        }

        std::cout << "\n\nvarianceOFstd:\n";
        for( size_t a = 0; a < variances_of_std_.size(); ++a )
        {
            for( size_t b = 0; b < variances_of_std_.size(); ++b )
                std::cerr << "\t" << variances_of_std_[a][b];
            std::cerr << "\n";
        }
    }

    void print_test()
    {
        for( size_t i = 0; i < buffers.size(); ++i )
        {
            std::cerr << "\n\nsample" << i << "\t" << buffers[i].boundarys.size()+1 << " clusters\nmean";

            for( auto& mm : buffers[i].means_of_mean )
                std::cerr << "\t" << mm;

            std::cerr << "\nstd";

            for( auto& ss : buffers[i].means_of_std )
                std::cerr << "\t" << ss;
        }

        std::cerr << "\n\n";
    }

    std::vector< BRLMMpBufferType > buffers;

    BRLMMp( const size_t& cluster_k, const std::string& brlmmp_type ) 
    : k_cluster_( cluster_k )
    , brlmmp_type_( brlmmp_type )
    , means_of_mean_( k_cluster_, 0 )
    , means_of_std_( k_cluster_, 0 )
    , variances_of_mean_({ means_of_mean_, means_of_mean_, means_of_mean_ })
    , variances_of_std_({ means_of_std_, means_of_std_, means_of_std_ })
    , bic_buffer_idxs_()
    , buffers()
    {
        check_cluster_k( this->k_cluster_ );
        set_bic_buffer_idxs( this->bic_buffer_idxs_ );
    }

    BRLMMp( const BRLMMp& brlmmp ) 
    : k_cluster_( brlmmp.k_cluster_ )
    , brlmmp_type_( brlmmp.brlmmp_type_ )
    , means_of_mean_( brlmmp.means_of_mean_ )
    , means_of_std_( brlmmp.means_of_std_ )
    , variances_of_mean_( brlmmp.variances_of_mean_ )
    , variances_of_std_( brlmmp.variances_of_std_ )
    , bic_buffer_idxs_( brlmmp.bic_buffer_idxs_ )
    , buffers( brlmmp.buffers )
    {
        check_cluster_k( this->k_cluster_ );
        set_bic_buffer_idxs( this->bic_buffer_idxs_ );
    }

    std::vector< double > outlier_cluster( const size_t& missing_k, BRLMMpBufferType& buffer )
    {
        std::vector< double > data;

        switch( missing_k )
        {
            case 0:
                for( size_t i = 0; i < buffer.data.size(); ++i )
                {
                    data.emplace_back( buffer.data[i] );
                }
                break;

            case 1:
                for( size_t i = 0; i < buffer.boundarys[0]; ++i )
                {
                    data.emplace_back( buffer.data[i] );
                }
                break;

            case 3:
                for( size_t i = buffer.boundarys[0]; i < buffer.data.size(); ++i )
                {
                    data.emplace_back( buffer.data[i] );
                }
                break;
        }

        return data;
    }

    double get_median( const std::vector< double >& data )
    {
        double median( 0.0 );

        if( data.size() % 2 == 0 )
        {
            median = ( data[( data.size() /2 )] + data[( data.size() /2 ) -1 ] ) / 2;
        }
        else
        {
            median = data[(( data.size() -1 ) / 2 )];
        }

        return median;
    }

    double gaussian_median_absolute_deviation( const double& median, const std::vector< double >& data )
    {
        std::vector< double > med_vec;

        for( auto& value : data )
        {
            med_vec.emplace_back( std::abs( value - median ));
        }

        std::sort( med_vec.begin(), med_vec.end() );

        return get_median( med_vec ) * 1.4826;
    }

    std::vector< size_t > get_outlier_idx( const std::vector< double >& data, const double& stdrange_1, const double& stdrange_2 )
    {
        std::vector< size_t > outlier_idx;

        for( size_t i = 0; i < data.size(); ++i )
        {
            if( data[i] < stdrange_1 || data[i] > stdrange_2 )
                outlier_idx.emplace_back( i );
        }

        return outlier_idx;
    }

    std::vector< double > remove_outlier( const size_t& missing_k, std::vector< size_t >& outlier_idx, const BRLMMpBufferType& buffer )
    {
        std::vector< size_t >::iterator it;
        std::vector< double > data;

        switch( missing_k )
        {
            case 0:
                for( size_t i = 0; i < buffer.data.size(); ++i )
                {
                    it = std::find( outlier_idx.begin(), outlier_idx.end(), i );

                    if( it == outlier_idx.end() )
                    {
                        data.emplace_back( buffer.data[i] );
                    }
                }
                break;

            case 1:
                for( size_t i = 0; i < buffer.boundarys[0]; ++i )
                {
                    it = std::find( outlier_idx.begin(), outlier_idx.end(), i );

                    if( it == outlier_idx.end() )
                    {
                        data.emplace_back( buffer.data[i] );
                    }
                }
                for( size_t i = buffer.boundarys[0]; i < buffer.data.size(); ++i )
                {
                    data.emplace_back( buffer.data[i] );
                }
                break;

            case 3:
                for( size_t i = 0; i < buffer.boundarys[0]; ++i )
                {
                    data.emplace_back( buffer.data[i] );
                }
                for( size_t i = buffer.boundarys[0]; i < buffer.data.size(); ++i )
                {
                    it = std::find( outlier_idx.begin(), outlier_idx.end(), i - buffer.boundarys[0] );

                    if( it == outlier_idx.end() )
                    {
                        data.emplace_back( buffer.data[i] );
                    }
                }
                break;
        }

        return data;
    }

    void outlier_trimming_for_1_cluster( BRLMMpBufferType& buffer )
    {
        std::vector< double > data( outlier_cluster( 0, buffer ));

        double median( get_median( data ));
        double std2( gaussian_median_absolute_deviation( median, data ) *2 );

        double stdrange_1 = median - std2;
        double stdrange_2 = median + std2;

        std::vector< size_t > outlier_idx( get_outlier_idx( data, stdrange_1, stdrange_2 ));

        if( !outlier_idx.empty() )
        {
            data = remove_outlier( 0, outlier_idx, buffer );
            BRLMMpTbl tbl( data, 1 );
            BRLMMpClustering brlmmp( data, 1, tbl);
            std::pair< double, BRLMMpBufferType > likelihood( brlmmp.run( this->brlmmp_type_ ));
            // buffer.clear();
            // buffer.swap( likelihood.second );
            buffer = std::move( likelihood.second );
        }
    }

    void outlier_trimming_for_2_cluster( BRLMMpBufferType& buffer )
    {
        double boundary_1 = buffer.means[0] + 1;
        double boundary_2 = 1 - buffer.means[1];
        std::vector< double > data;

        if( boundary_2 > boundary_1 )
        {
            data = outlier_cluster( 3, buffer );
        }
        else
        {
            data = outlier_cluster( 1, buffer );
        }

        double median( get_median( data ));
        double std2( gaussian_median_absolute_deviation( median, data ) *2 );

        double stdrange_1 = median - std2;
        double stdrange_2 = median + std2;

        std::vector< size_t > outlier_idx( get_outlier_idx( data, stdrange_1, stdrange_2 ));

        if( !outlier_idx.empty() )
        {
            if( boundary_2 > boundary_1 )
            {
                data = remove_outlier( 3, outlier_idx, buffer );
            }
            else
            {
                data = remove_outlier( 1, outlier_idx, buffer );
            }

            BRLMMpTbl tbl( data, 2 );
            BRLMMpClustering brlmmp( data, 2, tbl );
            std::pair< double, BRLMMpBufferType > likelihood( brlmmp.run( this->brlmmp_type_ ));
            // buffer.clear();
            // buffer.swap( likelihood.second );
            buffer = std::move( likelihood.second );
        }
    }

    void outlier_trimming_for_3_cluster(
              std::vector< std::pair< double, size_t >>& bic_vec
            , std::vector< std::pair< double, BRLMMpBufferType >>& likelihoods
            , const size_t& k
    )
    {
        std::vector< double > centers;

        for( auto& center : likelihoods[2].second.means )
        {
            centers.emplace_back( tan( 3.14159265 * center / 2 ));
        }

        double dis_compare = 0;
        double dist_1 = centers[1] - centers[0];
        double dist_2 = centers[2] - centers[1];

        if( dist_1 < dist_2 )
        {
            dis_compare = dist_2 / dist_1;
        }
        else
        {
            dis_compare = dist_1 / dist_2;
        }

        if( dis_compare > k )
        {
            BRLMMpTbl tbl( likelihoods[2].second.data, 2 );
            BRLMMpClustering brlmmp( likelihoods[2].second.data, 2, tbl );
            std::pair< double, BRLMMpBufferType > likelihood( brlmmp.run( this->brlmmp_type_ ));
            // likelihoods[2].second.clear();
            // likelihoods[2].second.swap( likelihood.second );
            likelihoods[2].second = std::move( likelihood.second );

            bic_vec[0].second = bic_vec[0].second -1;
        }

        // std::vector< double > data;

        // for( auto& center : likelihoods[2].second.means )
        // {
        //     data.push_back( center );
        // }

        // double median( get_median( data ));
        // double std( gaussian_median_absolute_deviation( median, data ));

        // double stdrange_1 = median - std;
        // double stdrange_2 = median + std;

        // if( ( data[0] < stdrange_1 && data[2] - data[1] < 0.33 )||
        //     ( data[2] > stdrange_2 && data[1] - data[0] < 0.33 ))
        // {
        //     BRLMMpClustering brlmmp( likelihoods[2].second.data, 2 );
        //     std::pair< double, BRLMMpBufferType > likelihood( brlmmp.run() );
        //     likelihoods[2].second.clear();
        //     likelihoods[2].second.swap( likelihood.second );

        //     bic_vec[0].second = bic_vec[0].second -1;
        // }
    }

    void outlier_trimmer(
              const std::vector< std::pair< double, size_t >>& bic_vec
            , std::vector< std::pair< double, BRLMMpBufferType >>& likelihoods
            // , const size_t& k = 4
            // , const double& percentage
    )
    {
        double boundary_1 = likelihoods[1].second.means[0] + 1;
        double boundary_2 = 1 - likelihoods[1].second.means[1];

        switch( bic_vec[0].second )
        {
            case 0:
                outlier_trimming_for_1_cluster( likelihoods[0].second );
                break;

            case 1:
                outlier_trimming_for_2_cluster( likelihoods[1].second );

                // if( boundary_2 > boundary_1 )
                // {
                //     model_trimming( 1, likelihoods[2].second, percentage );
                // }
                // else if( boundary_2 < boundary_1 )
                // {
                //     model_trimming( 0, likelihoods[2].second, percentage );
                // }

                break;

            case 2:
                // outlier_trimming_for_3_cluster( bic_vec, likelihoods, k );
                // model_trimming( 1, likelihoods[2].second, percentage );
                break;
        }
    }

    void get_start_end( size_t& start, size_t& end, size_t& k, BRLMMpBufferType& buffer )
    {
        switch( k )
        {
            case 0:
                if( buffer.n_cluster != 1 )
                {
                    end = buffer.boundarys[0];
                }
                break;

            case 1:
                switch( buffer.n_cluster )
                {
                    case 2:
                        start = buffer.boundarys[0];
                        end = buffer.data.size();
                        break;

                    case 3:
                        start = buffer.boundarys[0];
                        end = buffer.boundarys[1];
                        break;
                }
                break;

            case 2:
                start = buffer.boundarys[1];
                end = buffer.data.size();
                break;
        }
    }

    void boundary_mean_estimate( const size_t& idx, const size_t& start, const size_t& end, std::vector< double >& means, const std::vector< double >& datas )
    {
        double sum( 0.0 );

        for( size_t i = start; i < end; ++i )
        {
            sum += datas[ i ];
        }

        means[ idx ] = sum / ( end - start );
    }

    void NOINLINE boundary_variance_estimate( const size_t& idx, const size_t& start, const size_t& end, std::vector< double >& variances, const std::vector< double >& datas )
    {
        double sum( 0.0 );

        for( size_t i = start; i < end; ++i )
        {
            sum += datas[ i ];
        }

        double mean( sum / ( end - start ));
        sum = 0;

        for( size_t i = start; i < end; ++i )
        {
            sum += std::pow( datas[ i ] - mean, 2);
        }

        variances[ idx ] = sum / (( end - start ) - 1 );
    }

    void model_trimming( const size_t cluster_k, BRLMMpBufferType& buffer, const double& percentage )
    {
        size_t i = cluster_k;
        // for( size_t i = 0; i < buffer.n_cluster; ++i )
        // {
            std::vector< std::pair< size_t, double >> gaussian_vec;

            size_t start = 0;
            size_t end = buffer.data.size();

            get_start_end( start, end, i, buffer );

            for( size_t idx = start; idx < end; ++idx )
            {
                double p = boost::math::pdf(
                    boost::math::normal(
                        buffer.means[ i ],
                        sqrt( buffer.variances[ i ] )
                    ),
                    buffer.data[ idx ]
                );

                gaussian_vec.emplace_back( idx, p );
            }

            std::sort( gaussian_vec.begin(), gaussian_vec.end(),
                [ &gaussian_vec ]( std::pair< size_t, double >& a, std::pair< size_t, double >& b)
                {
                    return a.second > b.second;
                }
            );

            std::vector< double > datas;
            size_t trim_k = gaussian_vec.size() * percentage;

            if( trim_k > 2 )
            {
                for( size_t idx = start; idx < start + trim_k; ++idx )
                {
                    datas.push_back( buffer.data[ idx ] );
                }

                boundary_mean_estimate( i, 0, datas.size(), buffer.means, datas );
                boundary_variance_estimate( i, 0, datas.size(), buffer.variances, datas );
            }
        // }
    }

    double bic( const double& likelihood, const size_t& cluster_k, const size_t& data_n )
    {
        return( - 2 * likelihood + ( 3 * cluster_k - 1 ) * log( data_n ));
    }

    void find_min_bic( const std::vector< double >& data )
    {
        std::vector< std::pair< double, size_t >> bic_vec;
        std::vector< std::pair< double, BRLMMpBufferType >> likelihoods;

        for( size_t k = 0; k < this->k_cluster_; ++k )
        {
            BRLMMpTbl tbl(data, k+1);
            BRLMMpClustering brlmmp( data, k+1, tbl );
            likelihoods.emplace_back( brlmmp.run( this->brlmmp_type_ ));
        }

        for( size_t k = 0; k < likelihoods.size(); ++k )
        {
            bic_vec.emplace_back(
                bic( likelihoods[ k ].first, k+1, data.size() )
                , k
            );
        }

        std::sort( bic_vec.begin(), bic_vec.end(),
            [ &bic_vec ]( std::pair< double, size_t >& a, std::pair< double, size_t >& b )
            {
                return a.first < b.first;
            }
        );

        // outlier_trimmer( bic_vec, likelihoods ); XXX no percentage yet
        likelihoods[ bic_vec[0].second ].second.delete_data();

        this->buffers.emplace_back( std::move( likelihoods[ bic_vec[0].second ].second ));
        this->bic_buffer_idxs_[ bic_vec[0].second+1 ].emplace_back( this->buffers.size() -1 ); 
    }

    void bic_buffer_idxs_emplace( const size_t& k, const size_t& idx )
    {
        this->bic_buffer_idxs_[ k +1 ].emplace_back( idx -1 ); 
    }

    double incremental_mean_estimate( const double& last_mean, const double& this_n, const double& this_num )
    {
        return last_mean + ( this_n - last_mean ) / this_num;
    }

    void means_estimate_for_mean_or_std(
              const size_t& k
            , const size_t&n
            , std::vector< double >& mean_of_mean_or_std
            , const std::vector< double >& mean_or_std
            , std::vector< double >& buffer_mean_of_mean_or_std )
    {
        for( size_t i = 0; i < k; ++i )
        {
            mean_of_mean_or_std[i] = incremental_mean_estimate( mean_of_mean_or_std[i], mean_or_std[i], n );
        }

        switch( k )
        {
            case 3:
                buffer_mean_of_mean_or_std = mean_or_std;
        }
    }

    void variances_estimate_for_mean_or_std(
          const size_t& k
        , std::vector< std::vector< double >>& variance_of_mean_or_std
        , const std::vector< double >& mean_of_mean_or_std
        , const std::vector< double >& mean_or_std
    )
    {
        std::vector< double > vec;

        for( size_t i = 0; i < k; ++i )
        {
            vec.emplace_back( std::abs( mean_or_std[i] ) - std::abs( mean_of_mean_or_std[i] ));
        }

        for( size_t a = 0; a < k; ++a )
        {
            for( size_t b = 0; b < k; ++b )
            {
                variance_of_mean_or_std[a][b] += ( vec[a] * vec[b] );
            }
        }
    }

    void build_model( std::vector< BRLMMpBufferType >& buffer_vec )
    {
        build_model( this->k_cluster_, buffer_vec, this->bic_buffer_idxs_ );
    }

    void build_model( const size_t& k, std::vector< BRLMMpBufferType >& buffer_vec, std::map< size_t, std::vector< size_t >>& bic_buffer_idxs )
    {
        for( size_t i = 0; i < bic_buffer_idxs[ k ].size(); ++i )
        {
            means_estimate_for_mean_or_std( k, i+1, this->means_of_mean_, buffer_vec[ bic_buffer_idxs[k][i] ].means, buffer_vec[ bic_buffer_idxs[k][i] ].means_of_mean );
            means_estimate_for_mean_or_std( k, i+1, this->means_of_std_, buffer_vec[ bic_buffer_idxs[k][i] ].stds, buffer_vec[ bic_buffer_idxs[k][i] ].means_of_std );
        }

        for( auto& mean_of_std : this->means_of_std_ )
        {
            mean_of_std = std::log( mean_of_std );
        }

        for( size_t i = 0; i < bic_buffer_idxs[ k ].size(); ++i )
        {
            variances_estimate_for_mean_or_std( k, this->variances_of_mean_, this->means_of_mean_, buffer_vec[ bic_buffer_idxs[k][i] ].means );

            for( auto& std : buffer_vec[ bic_buffer_idxs[k][i] ].stds )
            {
                std = std::log( std );
            }

            variances_estimate_for_mean_or_std( k, this->variances_of_std_, this->means_of_std_, buffer_vec[ bic_buffer_idxs[k][i] ].stds );
        }

        for( size_t a = 0; a < k; ++a )
        {
            for( size_t b = 0; b < k; ++b )
            {
                this->variances_of_mean_[a][b] = this->variances_of_mean_[a][b] / ( bic_buffer_idxs[ k ].size() -1 );
                this->variances_of_std_[a][b] = this->variances_of_std_[a][b] / ( bic_buffer_idxs[ k ].size() -1 );
            }
        }

        for( size_t i = 0; i < bic_buffer_idxs[ k ].size(); ++i )
        {
            for( auto& std : buffer_vec[ bic_buffer_idxs[k][i] ].stds )
            {
                std = std::exp( std );
            }
        }

        for( auto& mean_of_std : this->means_of_std_ )
        {
            mean_of_std = std::exp( mean_of_std );
        }
    }

    void imputate_for_1_cluster( const size_t& missing_k, const std::vector< double >& mean_or_std, std::vector< double >& mean_of_mean_or_std )
    {
        std::vector< std::vector< double >> pow_of_variances_of_mean( this->variances_of_mean_ );

        for( size_t a = 0; a < pow_of_variances_of_mean.size(); ++a )
        {
            for( size_t b = 0; b < pow_of_variances_of_mean[a].size(); ++b )
            {
                pow_of_variances_of_mean[a][b] = std::pow( pow_of_variances_of_mean[a][b], 2 );
            }
        }
        
        imputate_for_1_cluster( missing_k, this->means_of_mean_, pow_of_variances_of_mean, mean_or_std, mean_of_mean_or_std );
    }

    void imputate_for_1_cluster(
          const size_t& missing_k
        , const std::vector< double >& grand_mean_of_mean_or_std
        , const std::vector< std::vector< double >>& grand_variance_of_mean_or_std
        , const std::vector< double >& mean_or_std
        , std::vector< double >& mean_of_mean_or_std
    )
    {
        mean_of_mean_or_std = std::vector< double >( k_cluster_, 0 );

        switch( missing_k )
        {
            case 1:
                mean_of_mean_or_std[0] = grand_mean_of_mean_or_std[0] + (
                                            ( mean_or_std[0] - grand_mean_of_mean_or_std[1] ) * ( 
                                                grand_variance_of_mean_or_std[0][1] * grand_variance_of_mean_or_std[2][2] -
                                                grand_variance_of_mean_or_std[0][2] * grand_variance_of_mean_or_std[2][1]
                                            ) +
                                            ( mean_or_std[1] - grand_mean_of_mean_or_std[2] ) * (
                                                grand_variance_of_mean_or_std[0][2] * grand_variance_of_mean_or_std[1][1] -
                                                grand_variance_of_mean_or_std[0][1] * grand_variance_of_mean_or_std[1][2]
                                            )
                                        ) /
                                        ( grand_variance_of_mean_or_std[1][1] * grand_variance_of_mean_or_std[2][2] -
                                          grand_variance_of_mean_or_std[1][2] * grand_variance_of_mean_or_std[2][1]
                                        );
                mean_of_mean_or_std[1] =   mean_or_std[0];
                mean_of_mean_or_std[2] =   mean_or_std[1];
                break;

            case 3:
                mean_of_mean_or_std[0] =   mean_or_std[0];
                mean_of_mean_or_std[1] =   mean_or_std[1];
                mean_of_mean_or_std[2] = grand_mean_of_mean_or_std[2] + (
                                            ( mean_or_std[0] - grand_mean_of_mean_or_std[0] ) * ( 
                                                grand_variance_of_mean_or_std[2][0] * grand_variance_of_mean_or_std[1][1] -
                                                grand_variance_of_mean_or_std[2][1] * grand_variance_of_mean_or_std[1][0]
                                            ) +
                                            ( mean_or_std[1] - grand_mean_of_mean_or_std[1] ) * (
                                                grand_variance_of_mean_or_std[2][1] * grand_variance_of_mean_or_std[0][0] -
                                                grand_variance_of_mean_or_std[2][0] * grand_variance_of_mean_or_std[0][1]
                                            )
                                        ) /
                                        ( grand_variance_of_mean_or_std[0][0] * grand_variance_of_mean_or_std[1][1] -
                                          grand_variance_of_mean_or_std[0][1] * grand_variance_of_mean_or_std[1][0]
                                        );
                break;

            default:
                throw std::runtime_error( "Error clustering imputation for missing cluster " + std::to_string( missing_k ) + "\n" );
        }
    }

    void imputate_for_2_cluster( const size_t& given_k, const std::vector< double >& mean_or_std, std::vector< double >& mean_of_mean_or_std )
    {
        std::vector< std::vector< double >> pow_of_variances_of_mean( this->variances_of_mean_ );

        for( size_t a = 0; a < pow_of_variances_of_mean.size(); ++a )
        {
            for( size_t b = 0; b < pow_of_variances_of_mean[a].size(); ++b )
            {
                pow_of_variances_of_mean[a][b] = std::pow( pow_of_variances_of_mean[a][b], 2 );
            }
        }

        imputate_for_2_cluster( given_k, this->means_of_mean_, pow_of_variances_of_mean, mean_or_std, mean_of_mean_or_std );
    }

    void imputate_for_2_cluster(
          const size_t& given_k
        , const std::vector< double >& grand_mean_of_mean_or_std
        , const std::vector< std::vector< double >>& grand_variance_of_mean_or_std
        , const std::vector< double >& mean_or_std
        , std::vector< double >& mean_of_mean_or_std
    )
    {
        mean_of_mean_or_std = std::vector< double >( k_cluster_, 0 );

        switch( given_k )
        {
            case 1:
                mean_of_mean_or_std[0] =   mean_or_std[0];
                mean_of_mean_or_std[1] =   grand_mean_of_mean_or_std[1]
                                       +   grand_variance_of_mean_or_std[1][0] / grand_variance_of_mean_or_std[0][0]
                                       * ( mean_or_std[0] - grand_mean_of_mean_or_std[0] );
                mean_of_mean_or_std[2] =   grand_mean_of_mean_or_std[2]
                                       +   grand_variance_of_mean_or_std[2][0] / grand_variance_of_mean_or_std[0][0]
                                       * ( mean_or_std[0] - grand_mean_of_mean_or_std[0] );
                break;

            case 3:
                mean_of_mean_or_std[0] =   grand_mean_of_mean_or_std[0]
                                       +   grand_variance_of_mean_or_std[0][2] / grand_variance_of_mean_or_std[2][2]
                                       * ( mean_or_std[0] - grand_mean_of_mean_or_std[2] );
                mean_of_mean_or_std[1] =   grand_mean_of_mean_or_std[1]
                                       +   grand_variance_of_mean_or_std[1][2] / grand_variance_of_mean_or_std[2][2]
                                       * ( mean_or_std[0] - grand_mean_of_mean_or_std[2] );
                mean_of_mean_or_std[2] =   mean_or_std[0];
                break;

            default:
                throw std::runtime_error( "Error clustering imputation for given cluster " + std::to_string( given_k ) + "\n" );
        }
    }

    void imputate_1_missing_cluster( std::vector< BRLMMpBufferType >& buffer_vec )
    {
        imputate_1_missing_cluster( buffer_vec, this->bic_buffer_idxs_ );
    }

    void imputate_1_missing_cluster( std::vector< BRLMMpBufferType >& buffer_vec, std::map< size_t, std::vector< size_t >>& bic_buffer_idxs )
    {
        std::vector< std::vector< double >> pow_of_variances_of_mean( this->variances_of_mean_ );

        for( size_t a = 0; a < pow_of_variances_of_mean.size(); ++a )
        {
            for( size_t b = 0; b < pow_of_variances_of_mean[a].size(); ++b )
            {
                pow_of_variances_of_mean[a][b] = std::pow( pow_of_variances_of_mean[a][b], 2 );
            }
        }

        for( size_t i = 0; i < bic_buffer_idxs[2].size(); ++i )
        { 
            double boundary_1 = buffer_vec[ bic_buffer_idxs[2][i] ].means[0] + 1;
            double boundary_2 = 1 - buffer_vec[ bic_buffer_idxs[2][i] ].means[1];

            if( boundary_2 > boundary_1 )
            {
                imputate_for_1_cluster(
                      3
                    , this->means_of_mean_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[2][i] ].means
                    , buffer_vec[ bic_buffer_idxs[2][i] ].means_of_mean
                );

                imputate_for_1_cluster(
                      3
                    , this->means_of_std_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[2][i] ].stds
                    , buffer_vec[ bic_buffer_idxs[2][i] ].means_of_std
                );
            }
            else if( boundary_2 < boundary_1 )
            {
                imputate_for_1_cluster(
                      1
                    , this->means_of_mean_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[2][i] ].means
                    , buffer_vec[ bic_buffer_idxs[2][i] ].means_of_mean
                );

                imputate_for_1_cluster(
                      1
                    , this->means_of_std_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[2][i] ].stds
                    , buffer_vec[ bic_buffer_idxs[2][i] ].means_of_std
                );
            }
            // else if( boundary_2 == boundary_1 )
            // {
            // }
            else {}
        }
    }

    void imputate_2_missing_cluster( std::vector< BRLMMpBufferType >& buffer_vec )
    {
        imputate_2_missing_cluster( buffer_vec, this->bic_buffer_idxs_ );
    }

    void imputate_2_missing_cluster( std::vector< BRLMMpBufferType >& buffer_vec, std::map< size_t, std::vector< size_t >>& bic_buffer_idxs )
    {
        std::vector< std::vector< double >> pow_of_variances_of_mean( this->variances_of_mean_ );

        for( size_t a = 0; a < pow_of_variances_of_mean.size(); ++a )
        {
            for( size_t b = 0; b < pow_of_variances_of_mean[a].size(); ++b )
            {
                pow_of_variances_of_mean[a][b] = std::pow( pow_of_variances_of_mean[a][b], 2 );
            }
        }

        for( size_t i = 0; i < bic_buffer_idxs[1].size(); ++i )
        { 
            double boundary_1 = buffer_vec[ bic_buffer_idxs[1][i] ].data_range.first + 1;
            double boundary_2 = 1 - buffer_vec[ bic_buffer_idxs[1][i] ].data_range.second;

            if( boundary_2 > boundary_1 )
            {
                imputate_for_2_cluster(
                      3
                    , this->means_of_mean_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[1][i] ].means
                    , buffer_vec[ bic_buffer_idxs[1][i] ].means_of_mean
                );

                imputate_for_2_cluster(
                      3
                    , this->means_of_std_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[1][i] ].stds
                    , buffer_vec[ bic_buffer_idxs[1][i] ].means_of_std
                );
            }
            else if( boundary_2 < boundary_1 )
            {
                imputate_for_2_cluster(
                      1
                    , this->means_of_mean_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[1][i] ].means
                    , buffer_vec[ bic_buffer_idxs[1][i] ].means_of_mean
                );

                imputate_for_2_cluster(
                      1
                    , this->means_of_std_
                    , pow_of_variances_of_mean
                    , buffer_vec[ bic_buffer_idxs[1][i] ].stds
                    , buffer_vec[ bic_buffer_idxs[1][i] ].means_of_std
                );
            }
            // else if( boundary_2 == boundary_1 )
            // {
            // }
            else {}
        }
    }

    BRLMMpTrainingType output_training( const std::vector< BRLMMpBufferType >& buffer_vec )
    {
        BRLMMpTrainingType res;

        for( auto& buffer : buffer_vec )
        {
            res.emplace_back( buffer.means_of_mean, buffer.means_of_std );
        }

        return res;
    }

    void output_training( const std::vector< BRLMMpBufferType >& buffer_vec, const std::string& path )
    {
        std::ofstream output( path );

        output << "Idx\tClusters...\tMean|Std\n";

        for( size_t idx = 0; idx < buffer_vec.size(); ++idx )
        {
            output << idx;

            for( size_t i = 0; i < this->k_cluster_; ++i )
            {
                output << "\t" << buffer_vec[ idx ].means_of_mean[i] << "|" << buffer_vec[ idx ].means_of_std[i];
            }
            
            output << "\n";
        }

        output.close();
    }

    void set_training( BRLMMpTrainingType& trainings )
    {
        for( auto& training : trainings )
        {
            BRLMMpBufferType buffer;

            buffer.means_of_mean = training.first;
            buffer.means_of_std = training.second;

            buffers.emplace_back( buffer );
        }

        trainings.clear();
    }

    void set_training( const std::string& path )
    {
        std::ifstream intput( path );
        std::string line;

        while( std::getline( intput, line ))
        {
            std::vector< std::string > clusters;
            boost::iter_split( clusters, line, boost::algorithm::first_finder( "\t" ));

            if( clusters[0] == "Idx" )
                continue;

            std::vector< double > means, stds;

            for( size_t i = 1; i < clusters.size(); ++i )
            {
                std::vector< std::string > vec;
                boost::iter_split( vec, clusters[i], boost::algorithm::first_finder( "|" ));

                std::stringstream ss_mean( vec[0] );
                std::stringstream ss_std( vec[1] );

                double mean, std;

                ss_mean >> mean;
                ss_std >> std;

                means.emplace_back( mean );
                stds.emplace_back( std );
            }

            BRLMMpBufferType buffer;

            buffer.means_of_mean = means;
            buffer.means_of_std = stds;

            buffers.emplace_back( buffer );
        }
    }

    std::pair< int16_t, double > genotyping( const size_t& idx, const double& data, const std::vector< BRLMMpBufferType >& buffer_vec, const double& cutoff )
    {
        return genotyping( this->k_cluster_, idx, data, buffer_vec, cutoff );
    }

    std::pair< int16_t, double > genotyping( const size_t& k_cluster, const size_t& idx, const double& data, const std::vector< BRLMMpBufferType >& buffer_vec, const double& cutoff )
    {
        std::vector< std::pair< int16_t,  double >> res;
        // std::vector< double > p_vec;
        double sum( 0.0 );

        for( size_t k = 0; k < k_cluster; ++k )
        {
            double p = boost::math::pdf(
                    boost::math::normal(
                        buffer_vec[ idx ].means_of_mean[ k ],
                        buffer_vec[ idx ].means_of_std[ k ]
                        ),
                    data
            );

            sum += p;
            res.emplace_back( k, ( p * 1/3 ));
            // p_vec.emplace_back( p );
        }

        for( auto& p : res )
        {
            p.second = p.second / sum;
        }

        // std::vector< size_t > index = sort( res );

        std::sort( res.begin(), res.end(),
            [ &res ]( std::pair< int16_t, double >& a, std::pair< int16_t, double >& b)
            {
                return a.second > b.second;
            }
        );

        double confidence = 1 - res[0].second;

        if( confidence < cutoff )
        {
            return std::make_pair( -1, confidence );
        }

        //XXX this is a do for Affy comparison
        {
            if( res[0].first == 0 )
            {
                return std::make_pair( 2, confidence );
            }

            if( res[0].first == 2 )
            {
                return std::make_pair( 0, confidence );
            }
        }

        return std::make_pair( res[0].first, confidence );
    }

    std::vector< size_t > sort( std::vector< std::pair< int16_t, double >>& vec )
    {
        std::vector< size_t > idx( vec.size() );
        std::iota( idx.begin(), idx.end(), 0 );

        std::sort( idx.begin(), idx.end(),
            [ &vec ]( size_t& a, size_t& b )
            {
                return vec[a].second > vec[b].second;
            }
            );

        std::sort( vec.begin(), vec.end(),
            [ &vec ]( std::pair< int16_t, double >& a, std::pair< int16_t, double >& b)
            {
                return a.second > b.second;
            }
        );

        return idx;
    }
};

} // namespace algorithm
} // namespace cpt
#endif
