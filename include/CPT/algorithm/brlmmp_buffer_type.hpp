#pragma once
#include <vector>
#include <stdexcept>
#include <CPT/utility.hpp>
namespace cpt {
namespace algorithm {

struct BRLMMpBufferType
{
    bool is_setup;
    std::vector< double > data; // TODO store idx
    std::size_t n_cluster;
    std::vector< double > means;
    std::vector< double > stds;
    std::vector< double > thetas;
    std::vector< double > variances;
    std::vector< double > likelihoods;
    std::vector< std::size_t > boundarys;
    std::pair< double, double > data_range;
    std::vector< double > means_of_mean;
    std::vector< double > means_of_std;
    std::vector< std::vector< double >> variances_of_mean;
    std::vector< std::vector< double >> variances_of_std;

    void setup( const std::vector< double >& data, const std::size_t& cluster_n )
    {
        if( is_setup )
        {
            throw std::runtime_error( "This BRLMMpBufferType object has been setup, it can't be setup again\n" );
        }

        BRLMMpBufferType buffer( data, cluster_n );
        this->swap( buffer );
    }

    BRLMMpBufferType() 
        : is_setup( false )
        , n_cluster()
        , means()
        , stds()
        , thetas()
        , variances()
        , likelihoods()
        , boundarys()
        , data_range()
        , means_of_mean()
        , means_of_std()
        , variances_of_mean()
        , variances_of_std()
    {
    }
    
    BRLMMpBufferType( const std::vector< double >& datas, const size_t& cluster_n ) 
        : is_setup( true )
        , data( std::move( datas ))
        , n_cluster( cluster_n )
        , means( n_cluster, 0.0 )
        , stds( n_cluster, 0.0 )
        , thetas( n_cluster, 0.0 )
        , variances( n_cluster, 0.0 )
        , likelihoods( n_cluster, 0.0 )
        , boundarys( n_cluster -1, 0 )
        , data_range( std::make_pair( data[0], data[ data.size()-1 ]))
        , means_of_mean( n_cluster, 0.0 )
        , means_of_std( n_cluster, 0.0 )
        , variances_of_mean({ means_of_mean, means_of_mean, means_of_mean})
        , variances_of_std({ means_of_std, means_of_std, means_of_std })
    {
    }
    void reset( const std::vector< double >& datas, const size_t& cluster_n ) 
    {
        data = std::move( datas );
        n_cluster = cluster_n;
        cpt::reset_vector ( means        , cluster_n      , 0.0            );
        cpt::reset_vector ( stds         , cluster_n      , 0.0            );
        cpt::reset_vector ( thetas       , cluster_n      , 0.0            );
        cpt::reset_vector ( variances    , cluster_n      , 0.0            );
        cpt::reset_vector ( likelihoods  , cluster_n      , 0.0            );
        cpt::reset_vector ( boundarys    , cluster_n - 1  , std::size_t(0) );
        data_range  = std::make_pair( data[0], data[ data.size()-1 ]);
        cpt::reset_vector ( means_of_mean, cluster_n      , 0.0            );
        cpt::reset_vector ( means_of_std , cluster_n      , 0.0            );
        variances_of_mean = { means_of_mean, means_of_mean, means_of_mean };
        variances_of_std  = { means_of_std , means_of_std , means_of_std  };
        is_setup = true;
    }

    void swap( BRLMMpBufferType& buffer )
    {
        is_setup = std::move( buffer.is_setup );
        data.swap( buffer.data );
        n_cluster = std::move( buffer.n_cluster );
        means.swap( buffer.means );
        stds.swap( buffer.stds );
        thetas.swap( buffer.thetas );
        variances.swap( buffer.variances );
        likelihoods.swap( buffer.likelihoods );
        boundarys.swap( buffer.boundarys );
        data_range.swap( buffer.data_range );
        means_of_mean.swap( buffer.means_of_mean );
        means_of_std.swap( buffer.means_of_std );
        variances_of_mean.swap( buffer.variances_of_mean );
        variances_of_std.swap( buffer.variances_of_std );
    }

    void clear()
    {
        BRLMMpBufferType buffer;
        this->swap( buffer );
    }

    void delete_data()
    {
        data.clear();
    }
};
}}
