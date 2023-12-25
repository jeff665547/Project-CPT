#pragma once
#include <cmath>
#include <CPT/format/cube.hpp>

namespace cpt {
namespace algorithm {

template< class TYPE = double >
class Transformations
{
    static_assert (
        std::is_floating_point< TYPE >::value,
        "Transformations< TYPE >: TYPE is not float point\n"
    );

    cpt::format::Cube< TYPE >& cube_;

public:

    /*
     *  MA plotting transformaion
     *  X = log2( a ) - log2( b )
     *  Y = ( log2( a ) + log2( b )) / 2
     */
    void log_ratio_strength( double& a, double& b )
    {
        if( a == 0 )
        {
            a = 1e-12;
        }

        if( b == 0 )
        {
            b = 1e-12;
        }

        double x = ( std::log( a )/std::log( 2 )) - ( std::log( b )/std::log( 2 ));
        double y = (( std::log( a )/std::log( 2 )) + ( std::log( b )/std::log( 2 ))) / 2;

        a = x;
        b = y;
    }
    
    /*
     *  Coordinate Theta Transformations in R package
     *  Transforms between cartesian, spherical, polar, and cylindrical coordinate systems in two or three dimensions.
     *  In this function only provide 2 dimensions transformation into theta and rho.
     *  The algorithms in the MatLab and R
     *      Thera = atan2( y / x )
     *      Rho = sqrt( x^2 + y^2 )
     *  -----------------
     *  |      * point  |
     *  |   o /         |
     *  |  h /          |
     *  | r /           |
     *  |  /            |
     *  | / ) theta     |
     *  -----------------
     */
    void r_theta( double& a, double& b )
    {
        double x = std::atan( a / b ) - 0.7853982;  // To center the theta at zero

        if( b == 0 )
        {
            b = 1e-12;
        }
        
        double y = std::log( std::sqrt(( a * a ) + ( b * b ) + 1e-12 ));

        a = x;
        b = y;
    }

    /*
     *  X = asinh( K * ( A - B )/( A + B )) / asinh( K )
     *  Y = log2( A + B )
     */
    void contrast_centers_stretch( const double& k, double& a, double& b )
    {
        if( a + b == 0 )
        {
            throw std::runtime_error(
                "Error in Transformations::CCS(), A allele + B allele can't be zero\n"
            );
        }

        double r = k * ( a - b )/( a + b );
        r = std::log( r + std::sqrt(( r * r ) + 1 ));

        double x = r / std::log( k + std::sqrt(( k * k ) + 1 ));
        double y = ( std::log( a + b )/std::log( 2 ));

        a = x;
        b = y;
    }

    /*
     *  X = sinh( K * ( A - B )/( A + B )) / sinh( K )
     *  Y = log2( A + B )
     */
    void contrast_extremes_stretch( const double& k, double& a, double& b )
    {
        if( a + b == 0 )
        {
            throw std::runtime_error(
                "Error in Transformations::CES(), A allele + B allele can't be zero\n"
            );
        }

        double x = std::sinh( k * ( a - b )/ a + b )/ std::sinh( k );
        double y = ( std::log( a + b )/std::log( 2 ));

        a = x;
        b = y;
    }

    Transformations( cpt::format::Cube< TYPE >& cube )
        : cube_( cube )
    {
    }

    void mva(double& a, double& b)
    {
        log_ratio_strength(a, b);
    }

    void ma()
    {
        size_t cube_x = cube_.n_rows;
        size_t cube_y = cube_.n_cols;
        
        for( size_t j = 0; j < cube_y; ++j )
        {
            for( size_t i = 0; i < cube_x; ++i )
            {
                log_ratio_strength( cube_( i, j, 0 ), cube_( i, j, 1 ));
            }
        }
    }

    void rt()
    {
        size_t cube_x = cube_.n_rows;
        size_t cube_y = cube_.n_cols;

        for( size_t j = 0; j < cube_y; ++j )
        {
            for( size_t i = 0; i < cube_x; ++i )
            {
                r_theta( cube_( i, j, 0 ), cube_( i, j, 1 ));
            }
        }
    }

    void ccs( const double& k = 4.0 )
    {
        size_t cube_x = cube_.n_rows;
        size_t cube_y = cube_.n_cols;

        for( size_t j = 0; j < cube_y; ++j )
        {
            for( size_t i = 0; i < cube_x; ++i )
            {
                contrast_centers_stretch( k, cube_( i, j, 0 ), cube_( i, j, 1 ));
            }
        }
    }

    void ces( const double& k = 4 )
    {
        size_t cube_x = cube_.n_rows;
        size_t cube_y = cube_.n_cols;

        for( size_t j = 0; j < cube_y; ++j )
        {
            for( size_t i = 0; i < cube_x; ++i )
            {
                contrast_extremes_stretch( k, cube_( i, j, 0 ), cube_( i, j, 1 ));
            }
        }
    }
};

} // namespace algorithm
} // namespace cpt
