#pragma once
#include <CPT/format/cube.hpp>
#include <Nucleona/parallel/thread_pool.hpp>
//  Questions:
//  1.  Could this algorithm handle #channels > 2?
//  2.  Why use it like a wrapper?
//      I'm not sure this pattern is good or not.
//  3.  What is the purpose of scaling factor?

namespace cpt {
namespace algorithm {

template< typename TYPE = double >
class QuantileNormalizationForCube
{
    cpt::format::Cube< TYPE >& cube_;
    cpt::format::Cube< TYPE > sorted_cube_martix_;

    std::vector< TYPE >& sketch_table_;
    size_t scaling_factor_;

public:

    QuantileNormalizationForCube( cpt::format::Cube< TYPE >& cube, std::vector< TYPE >& sketch_table )
        : cube_( cube )
        , sketch_table_( sketch_table )
        , scaling_factor_( 50000 )
    {
        this->set_up();

        // if( sketch_table_.empty() )
        // {
        //     this->target_sketch_estimate();
        // }
        // else
        // {
        //     scaling_factor_ = sketch_table_.size();
        // }

        // this->do_quantile_normalization();
    }

    QuantileNormalizationForCube( 
        cpt::format::Cube< TYPE >& cube, 
        std::vector< TYPE >& sketch_table, 
        size_t scaling_factor, 
        size_t thread_num = 1 
    )
        : cube_( cube )
        , sketch_table_( sketch_table )
        , scaling_factor_( scaling_factor )
    {
        if( scaling_factor_ == 0 )
        {
            scaling_factor_ = cube_.n_rows;
        }

        this->set_up( thread_num );

        // if( sketch_table_.empty() )
        // {
        //     this->target_sketch_estimate();
        // }
        // else
        // {
        //     std::cerr << "Detect the exist \"sketch_table\", the quantile will use \"sketch_table\" and ignore the \"scaling_factor\"\n";
        //     scaling_factor_ = sketch_table_.size();
        // }

        // this->do_quantile_normalization();
    }

    void para_sort_by_col( cpt::format::Cube< TYPE >& cube, const size_t& thread_num )
    {
        size_t base_x = cube.n_rows;
        size_t base_y = cube.n_cols;
        size_t base_z = cube.n_slices;

        if( !cube.cube_index.is_empty() )
        {
            throw std::runtime_error( "You already have a temporary index store for the original order" );
        }

        arma::Cube< double > idx_cube( base_x, base_y, base_z );
        std::vector< TYPE > vec( base_x, 0 );

        std::vector< std::tuple< size_t, size_t, arma::Col< double >, std::vector< TYPE >>> tuple_vec; 
        //                          k      j            idx                 vec

        for( size_t k = 0; k < base_z; ++k )
        {
            for( size_t j = 0; j < base_y; ++j )
            {
                vec.resize( base_x, 0 );
                
                for( int i = 0; i < base_x; ++i )
                {
                    vec[ i ] = std::move( cube.at( i, j, k ));
                }

                tuple_vec.emplace_back( k, j, arma::Col< double >(), std::move( vec ));
                vec.clear();
            }
        }

        std::mutex mutex;
        auto parallel_pool( nucleona::parallel::make_thread_pool(thread_num ));

        std::vector< size_t > jobs;
        size_t thread_size = ( tuple_vec.size() / thread_num ) +1;

        for( size_t i = 0; i < tuple_vec.size(); ++i )
        {
            jobs.emplace_back( i );

            if( jobs.size() < thread_size )
            {
                if( i != tuple_vec.size()-1 )
                {
                    continue;
                }
            }

            parallel_pool.job_post( [ &tuple_vec, &mutex, jobs, this ] ()
            {
                std::vector< std::tuple< size_t, arma::Col< double >, std::vector< TYPE >>> temp_vec; 

                for( auto& id : jobs )
                {
                    temp_vec.emplace_back(
                          id
                        , std::move( arma::Col< double >( std::move( cpt::utility::sort( std::get<3>( tuple_vec[ id ] )))))
                        , std::move( std::get<3>( tuple_vec[ id ] ))
                    );
                }

                {
                    std::lock_guard< std::mutex > lock( mutex );

                    for( auto& temp : temp_vec )
                    {
                        std::get<2>( tuple_vec[ std::get<0>( temp ) ] ) = std::move( std::get<1>( temp ));
                        std::get<3>( tuple_vec[ std::get<0>( temp ) ] ) = std::move( std::get<2>( temp ));
                    }
                }
            });

            jobs.clear();
        }

        parallel_pool.flush();

        for( auto& tuple : tuple_vec )
        {
            idx_cube.slice( std::get<0>( tuple )).col( std::get<1>( tuple )) = std::move( std::move( std::get<2>( tuple )));
            cube.slice(     std::get<0>( tuple )).col( std::get<1>( tuple )) = std::move( arma::Col< TYPE >( std::move( std::get<3>( tuple ))));
        }

        cube.cube_index.resize(idx_cube.n_rows, idx_cube.n_cols, idx_cube.n_slices);
        cube.cube_index.swap( idx_cube );
    }

    /*
     *  Set things up for quntile normalization, this step include sorting
     */
    void set_up( size_t thread = 1 )
    {
        size_t cube_x = cube_.n_rows;
        size_t cube_y = cube_.n_cols * cube_.n_slices;

        cpt::format::Cube< TYPE > cube( cube_x, cube_y, 1 );
        cube.slice( 0 ) = cube_.as_matrix();

        // cube.sort_by_col();
        para_sort_by_col( cube, thread );

        sorted_cube_martix_.resize(cube.n_rows, cube.n_cols, cube.n_slices);
        sorted_cube_martix_.cube_index.resize(cube.cube_index.n_rows, cube.cube_index.n_cols, cube.cube_index.n_slices);
        sorted_cube_martix_.swap( cube );
    }

    /*
     *  Scaling the target sketch to fit the scaling facetor, this step include mean calculation
     */
    void scale_target_sketch( std::vector< TYPE >& mean_vec )
    {
        if( scaling_factor_ == 0 )
        {
            scaling_factor_ = cube_.n_rows;
        }

        size_t total = mean_vec.size();
        size_t rows = total / scaling_factor_;

        TYPE sum( 0 );
        std::vector< TYPE > scaled_vec, temp_vec, temp2_vec;

        for( size_t i = 0; i < total; ++i )
        {
            temp_vec.push_back( mean_vec[i] );

            if(( i + 1 )% rows == 0 )
            {
                for( auto& nu : temp_vec )
                {
                    sum += nu;
                }

                scaled_vec.push_back( double( sum ) / double( temp_vec.size() ));

                temp_vec.swap( temp2_vec );
                temp_vec.clear();
                sum = 0;
            }
        }

        if( scaled_vec.size() < rows )
        {
            for( auto& nu : temp_vec )
            {
                sum += nu;
            }

            scaled_vec.push_back( double( sum ) / double( temp_vec.size() ));
        }
        else
        {
            temp2_vec.insert( temp2_vec.end(), temp_vec.begin(), temp_vec.end() );

            for( auto& nu : temp2_vec )
            {
                sum += nu;
            }

            scaled_vec[ scaled_vec.size() -1 ] = double( sum ) / double( temp2_vec.size() );
        }

        mean_vec.swap( scaled_vec );
    }

    /*
     *  Do estimate for the target sketch
     */
    void target_sketch_estimate()
    {
        size_t cube_x = cube_.n_rows;

        std::vector< TYPE > mean_vec( cube_x );

        for( size_t i = 0; i < cube_x; ++i )
        {
            mean_vec[ i ] = sorted_cube_martix_.get_mean_by_row( i );
        }

        this->scale_target_sketch( mean_vec );
        sketch_table_ = std::move( mean_vec );
    }

    /*
     *  Do estimate for the target sketch by input scaling_factor
     */
    void target_sketch_estimate( size_t scaling_factor )
    {
        scaling_factor_= scaling_factor;
        target_sketch_estimate();
    }

    /*
     *  get target shetch
     */
    const std::vector< TYPE >& get_target_sketch() const
    {
        return sketch_table_;
    }

    /*
     *  Do quantile normalization, this step include replace and reorder
     */
    void do_quantile_normalization()
    {
        if( sketch_table_.empty() )
        {
            throw std::runtime_error(
                "The \"sketch_table_\" is empty, please do \"target_sketch_estimate\" first\n"
            );
        }

        size_t cube_x = cube_.n_rows;
        size_t cube_y = cube_.n_cols * cube_.n_slices;

        size_t base_y = cube_.n_cols;
        size_t base_z = cube_.n_slices;

        size_t rows = cube_x / scaling_factor_;
        size_t k = 0;

        for( size_t i = 0; i < cube_x; ++i )
        {
            if( i % rows == 0 && i != 0 && k < sketch_table_.size()-1 )
            {
                k++;
            }

            for( size_t j = 0; j < cube_y; ++j )
            {
                sorted_cube_martix_( i, j, 0 ) = sketch_table_[ k ];
            }
        }

        sorted_cube_martix_.reorder_cube_by_col();

        cpt::format::Cube< TYPE > cube( cube_x, base_y, base_z );
        cube.as_cube( sorted_cube_martix_.slice( 0 ), base_z );

        cube.reassign_axis( cube_.x_axis, cube_.y_axis, cube_.z_axis );
        cube.swap( cube_ );
    }
};

} // end of namespace algorithm
} // end of namespace cpt
