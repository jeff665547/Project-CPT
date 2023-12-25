#pragma once
#include <iostream>
#include <cmath>
#include <boost/math/distributions/normal.hpp>

namespace cpt {
namespace algorithm {

class RobustClusteringUtility
{
public:

    bool is_setup;
    size_t n_cluster;

    std::vector< double > means;
    std::vector< double > thetas;
    std::vector< double > variances;
    std::vector< double > likelihoods;
    std::vector< size_t > boundarys;

    void setup( const size_t& cluster_n )
    {
        if( is_setup )
        {
            throw std::runtime_error( "This RobustClusteringUtility object has been setup, it can't be setup again\n" );
        }

        RobustClusteringUtility utility( cluster_n );
        this->swap( utility );
    }

    RobustClusteringUtility() 
        : is_setup( false )
        , n_cluster()
        , means()
        , thetas()
        , variances()
        , likelihoods()
        , boundarys()
    {
    }

    RobustClusteringUtility( const size_t& cluster_n ) 
        : is_setup( true )
        , n_cluster( cluster_n )
        , means( n_cluster, 0.0 )
        , thetas( n_cluster, 0.0 )
        , variances( n_cluster, 0.0 )
        , likelihoods( n_cluster, 0.0 )
        , boundarys( n_cluster -1, 0 )
    {
    }

    RobustClusteringUtility( const RobustClusteringUtility& utility ) 
        : is_setup( utility.is_setup )
        , n_cluster( utility.n_cluster )
        , means( utility.means )
        , thetas( utility.thetas )
        , variances( utility.variances )
        , likelihoods( utility.likelihoods )
        , boundarys( utility.boundarys )
    {
    }

    RobustClusteringUtility( RobustClusteringUtility&& utility ) 
        : is_setup( std::move( utility.is_setup ))
        , n_cluster( std::move( utility.n_cluster ))
        , means( std::move( utility.means ))
        , thetas( std::move( utility.thetas ))
        , variances( std::move( utility.variances ))
        , likelihoods( std::move( utility.likelihoods ))
        , boundarys( std::move( utility.boundarys ))
    {
    }

    RobustClusteringUtility& operator=( const RobustClusteringUtility& utility ) 
    {
        is_setup = utility.is_setup;
        n_cluster = utility.n_cluster;
        means = utility.means;
        thetas = utility.thetas;
        variances = utility.variances;
        likelihoods = utility.likelihoods;
        boundarys = utility.boundarys;
        return *this;
    }

    RobustClusteringUtility& operator=( RobustClusteringUtility&& utility ) 
    {
        is_setup = std::move( utility.is_setup );
        n_cluster = std::move( utility.n_cluster );
        means = std::move( utility.means );
        thetas = std::move( utility.thetas );
        variances = std::move( utility.variances );
        likelihoods = std::move( utility.likelihoods );
        boundarys = std::move( utility.boundarys );
        return *this;
    }

    void swap( RobustClusteringUtility& utility )
    {
        is_setup = std::move( utility.is_setup );
        n_cluster = std::move( utility.n_cluster );
        means.swap( utility.means );
        thetas.swap( utility.thetas );
        variances.swap( utility.variances );
        likelihoods.swap( utility.likelihoods );
        boundarys.swap( utility.boundarys );
        return this->swap( utility );
    }

    void clear()
    {
        RobustClusteringUtility utility;
        this->swap( utility );
    }
};

template< class TYPE = double >
class RobustClustering
{
    static_assert (
        std::is_floating_point< TYPE >::value,
        "RobustClustering< TYPE >: TYPE is not float point\n"
    );

public:

    RobustClusteringUtility utility;
    const std::vector< double >& datas;

    std::vector< double > last_means;
    std::vector< double > last_variances;
    std::vector< size_t > last_boundarys;

    std::vector< std::pair< double, RobustClusteringUtility >> likelihoods;
    //                  Sum Likelihood

    bool is_border( const double& boundary )
    {
        if( boundary >= ( datas.size() ))
        {
            return true;
        }
        return false;
    }

    double likelihood_sum()
    {
        double res( 0.0 );

        for( auto& likelihood : utility.likelihoods )
        {
            res += likelihood;
            likelihood = 0;
        }

        return res;
    }

    void labelz_estimate()
    {
        utility.thetas[0] = double( utility.boundarys[0] ) / double( datas.size() );

        for( size_t i = 1; i < utility.boundarys.size(); ++i )
        {
            utility.thetas[i] = double( utility.boundarys[i] - utility.boundarys[ i-1 ] ) / double( datas.size() );
        }

        utility.thetas[ utility.boundarys.size() ] = double( datas.size() - utility.boundarys[ utility.boundarys.size() -1 ] ) / double( datas.size() );
    }

    void incremental_mean_estimate( const size_t& idx, const double& this_n, const double& this_num )
    {
        utility.means[ idx ] = last_means[ idx ] + ( this_n - last_means[ idx ] ) / this_num;
    }

    void decremental_mean_estimate( const size_t& idx, const double& last_n, const double& last_num )
    {
        utility.means[ idx ] = ( 1 / ( last_num -1 )) * (( last_num * last_means[ idx ] ) - last_n );
    }

    void boundary_mean_estimate( const size_t& idx, const size_t& start, const size_t& end )
    {
        double sum( 0.0 );

        for( size_t i = start; i < end; ++i )
        {
            sum += datas[ i ];
        }

        utility.means[ idx ] = sum / ( end - start );
    }

    void first_cluster_mean_estimate()
    {
        switch( utility.boundarys[0] )
        {
            case 0:
                utility.means[0] = 0;
                break;

            default:
                if( utility.boundarys[0] != last_boundarys[0] )
                {
                    incremental_mean_estimate( 0, datas[ utility.boundarys[0] -1 ], utility.boundarys[0] );
                }
                else {}
        }
    }

    void cluster_mean_estimate( const size_t& i )
    {
        if( utility.boundarys[i] == datas.size() ||
            utility.boundarys[i] - last_boundarys[i] < 0 ||
            utility.boundarys[i] == utility.boundarys[ i-1 ]
        )
        {
            utility.means[i] = 0;
            return;
        }
        else {}

        if( utility.boundarys[i] != last_boundarys[i] )
        {
            incremental_mean_estimate( i, datas[ utility.boundarys[i] -1 ], utility.boundarys[i] - utility.boundarys[ i-1 ] );
        }
        else {}
    }

    void last_cluster_mean_estimate()
    {
        if( utility.boundarys[ utility.boundarys.size() -1 ] == datas.size() )
        {
            utility.means[ utility.boundarys.size() ] = 0;
            return;
        }
        else {}

        if( utility.boundarys[ utility.boundarys.size() -1 ] > last_boundarys[ last_boundarys.size() -1 ] )
        {
            decremental_mean_estimate(
                    utility.boundarys.size(),
                    datas[ utility.boundarys[ utility.boundarys.size() -1 ] -1 ],
                    datas.size() - utility.boundarys[ utility.boundarys.size() -1 ] +1
                    );
        }
        else
        {
            boundary_mean_estimate(
                    utility.boundarys.size(),
                    utility.boundarys[ utility.boundarys.size() -1 ],
                    datas.size()
                    );
        }
    }

    void mean_estimate()
    {
        last_means = utility.means;

        first_cluster_mean_estimate();

        for( size_t i = 1; i < utility.boundarys.size(); ++i )
        {
            cluster_mean_estimate( i );
        }

        last_cluster_mean_estimate();
    }

    void incremental_variance_estimate( const size_t& idx, const double& this_n, const double& this_num )
    {
        utility.variances[ idx ] = ( this_num -2 ) / ( this_num -1 ) * last_variances[ idx ] + ( 1 / this_num ) * std::pow( last_means[ idx ] - this_n, 2 );
    }

    void decremental_variance_estimate( const size_t& idx, const double& last_n, const double& last_num )
    {
        utility.variances[ idx ] = ( last_num - 1 ) / ( last_num -2 ) * ( last_variances[ idx ] - ( 1 /  last_num ) * std::pow( utility.means[ idx ] - last_n, 2 ));
    }

    void boundary_variance_estimate( const size_t& idx, const size_t& start, const size_t& end )
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

        utility.variances[ idx ] = sum / (( end - start ) - 1 );
    }

    void first_cluster_variance_estimate()
    {
        switch( utility.boundarys[0] )
        {
            case 0:
                utility.variances[0] = 0;
                break;

            case 1:
                utility.variances[0] = 0;
                break;

            case 2:
                boundary_variance_estimate( 0, 0, utility.boundarys[0] );
                break;

            default:
                if( utility.boundarys[0] > last_boundarys[0] )
                {
                    incremental_variance_estimate( 0, datas[ utility.boundarys[0] -1 ], utility.boundarys[0] );
                }
                else {}
        }
    }

    void cluster_variance_estimate( const size_t& i )
    {
        if( utility.boundarys[i] == datas.size() ||
            utility.boundarys[i] < last_boundarys[i]
        )
        {
            utility.variances[i] = 0;
            return;
        }
        else {}

        switch( utility.boundarys[i] - utility.boundarys[ i-1 ] )
        {
            case 0:
                utility.variances[i] = 0;
                break;

            case 1:
                utility.variances[i] = 0;
                break;

            case 2:
                boundary_variance_estimate( i, utility.boundarys[ i-1 ], utility.boundarys[i] );
                break;

            default:
                if( utility.boundarys[i] > last_boundarys[i] )
                {
                    incremental_variance_estimate( i, datas[ utility.boundarys[i] -1 ], utility.boundarys[i] - utility.boundarys[ i-1 ] );
                }
                else {}
        }
    }

    void last_cluster_variance_estimate()
    {
        switch( datas.size() - utility.boundarys[ utility.boundarys.size() -1 ] )
        {
            case 0:
                utility.variances[ utility.boundarys.size() ] = 0;
                break;

            case 1:
                utility.variances[ utility.boundarys.size() ] = 0;
                break;

            default:
                if( utility.boundarys[ utility.boundarys.size() -1 ] > last_boundarys[ last_boundarys.size() -1 ] )
                {
                    decremental_variance_estimate(
                            utility.boundarys.size(),
                            datas[ utility.boundarys[ utility.boundarys.size() -1 ] -1 ],
                            datas.size() - utility.boundarys[ utility.boundarys.size() -1 ] +1
                            );
                }
                else
                {
                    boundary_variance_estimate(
                            utility.boundarys.size(),
                            utility.boundarys[ utility.boundarys.size() -1 ],
                            datas.size()
                            );
                }
        }
    }

    void variance_estimate()
    {
        last_variances = utility.variances;

        first_cluster_variance_estimate();

        for( size_t i = 1; i < utility.boundarys.size(); ++i )
        {
            cluster_variance_estimate( i );
        }

        last_cluster_variance_estimate();
    }

    double gaussian_update( const size_t& idx )
    {
        double res( 0.0 );

        for( size_t i = 0; i < utility.n_cluster; ++i )
        {
            if( utility.variances[ i ] == 0 )
            {
                continue;
            }

            double p = boost::math::pdf(
                    boost::math::normal(
                        utility.means[ i ],
                        sqrt( utility.variances[ i ] )
                        ),
                    datas[ idx ]
            );

            res += ( p * utility.thetas[ i ] );
        }

        return log( res );
    }

    void likelihood_estimate()
    {
        size_t i = 0;

        for( size_t j = 0; j < utility.boundarys.size(); ++j )
        {
            for( ; i < utility.boundarys[j]; ++i )
            {
                utility.likelihoods[j] += gaussian_update( i );
            }
        }

        for( ; i < datas.size(); ++i )
        {
            utility.likelihoods[ utility.likelihoods.size() -1 ] += gaussian_update( i );
        }

        likelihoods.emplace_back( std::make_pair( likelihood_sum(), utility ));
    }

    size_t get_max_likelihood()
    {
        size_t max_index( 0 );
        double value( likelihoods[ 0 ].first );

        for( size_t i = 0; i < likelihoods.size(); ++i )
        {
            const auto& get_value = likelihoods[ i ].first;

            if( value < get_value )
            {
                value = get_value;
                max_index = i;
            }
        }

        return max_index;
    }

    RobustClustering( const std::vector< TYPE >& data, const size_t& n_cluster ) 
        : utility( n_cluster )
        , datas( data )
        , last_means( n_cluster, 0 )
        , last_variances( n_cluster, 0 )
        , last_boundarys( n_cluster, 0 )
        , likelihoods()
    {
    }

    bool move_boundarys()
    {
        int i = utility.boundarys.size() -2;

        for( ; i >= 0; --i )
        {
            if( !is_border( utility.boundarys[i] ))
            {
                utility.boundarys[i]++;
                break;
            }
        }

        const auto& start_point = utility.boundarys[i];

        for( ; i < utility.boundarys.size(); ++i )
        {
            utility.boundarys[i] = start_point;
        }
    }

    bool sweeping()
    {
        // break_point();

        labelz_estimate();
        mean_estimate();
        variance_estimate();
        likelihood_estimate();

        last_boundarys = utility.boundarys;           

        // debug_output();

        if( is_border( utility.boundarys[0] ))
        {
            return false;
        }

        if( is_border( utility.boundarys[ utility.boundarys.size() -1 ] ))
        {
            move_boundarys();
        }
        else
        {
            utility.boundarys[ utility.boundarys.size() -1 ]++;
        }

        return true;
    }

    std::pair< double, RobustClusteringUtility > run()
    {
        while( true )
        {
            if( !sweeping() )
            {
                break;
            }
        }

        return likelihoods[ get_max_likelihood() ];
    }

    void break_point()
    {
        bool bp = false;

        if( utility.boundarys == std::vector< size_t >{ 0, 1 } )
            bp = true;
    }

    void debug_output()
    {
        std::cerr << "\nboundary:";

        for( auto& boundary : utility.boundarys )
        {
            std::cerr << " " << boundary;
        }

        std::cerr
            << "\tlikelihood:"
            << likelihoods[ likelihoods.size()-1 ].first
            ;

        std::cerr << "\tlabelz:";

        for( auto& labelz : utility.thetas )
        {
            std::cerr << " " << labelz;
        }

        std::cerr << "\tmean:";

        for( auto& mean : utility.means )
        {
            std::cerr << " " << mean;
        }

        std::cerr << "\tvariance:";

        for( auto& variance : utility.variances )
        {
            std::cerr << " " << variance;
        }
    }
};

} // namespace algorithm
} // namespace cpt
