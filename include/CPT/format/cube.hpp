#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <armadillo>
#include <CPT/format/cube_axis.hpp>
#include <CPT/utility/sort.hpp>
#include <boost/serialization/access.hpp>
//TODO new feature:
//     rearrange dimensions automatically by checking the order of label names

namespace cpt {
namespace format {

template< typename TYPE = double >
class Cube : public arma::Cube< TYPE >
{
    static_assert (
        std::is_floating_point< TYPE >::value,
        "Transformations< TYPE >: TYPE is not float point\n"
    );
  public:
    typedef arma::Cube< TYPE > Base;
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & (Base&)*this   ;
        ar & cube_index     ;
        ar & x_axis         ;
        ar & y_axis         ;
        ar & z_axis         ;
    }

  public:

    using Base::Base; // FIXME : ambiguous with constructor 2
    using Mat = arma::Mat< TYPE >;

    arma::Cube< double > cube_index;

    CubeAxis x_axis;
    CubeAxis y_axis;
    CubeAxis z_axis;

    Cube()
        : Base( 0, 0, 0, arma::fill::zeros )
        , cube_index( 0, 0, 0, arma::fill::zeros )
        , x_axis()
        , y_axis()
        , z_axis()
    {}

    Cube( const size_t x, const size_t y, const size_t z ) 
        // FIXME : constructor 2
        : Base( x, y, z, arma::fill::zeros )
        , cube_index( 0, 0, 0, arma::fill::zeros )
        , x_axis()
        , y_axis()
        , z_axis()
    {}

    Cube( const Cube& cube )
        : Base( cube )
        , cube_index( cube.cube_index )
        , x_axis( cube.x_axis )
        , y_axis( cube.y_axis )
        , z_axis( cube.z_axis )
    {}

    Cube& operator=( const Cube& cube )
    {
        this = cube;
        cube_index = cube.cube_index;
        x_axis = cube.x_axis;
        y_axis = cube.y_axis;
        z_axis = cube.z_axis;
        return *this;
    }

    Cube( Cube&& cube )
        : Base( std::move( cube )) 
        , cube_index( std::move( cube.cube_index ))
        , x_axis( std::move( cube.x_axis ))
        , y_axis( std::move( cube.y_axis ))
        , z_axis( std::move( cube.z_axis ))
    {}

    Cube& operator=( Cube&& cube )
    {
        Base::swap( cube );
        cube_index = std::move( cube.cube_index );
        x_axis = std::move( cube.x_axis );
        y_axis = std::move( cube.y_axis );
        z_axis = std::move( cube.z_axis );
        return *this;
    }

  private:

    bool is_equal( const arma::Cube< double >& cube_a, const arma::Cube< double >& cube_b ) const
    {
        bool check( true );

        if( cube_a.empty() && cube_b.empty() )
        {
            return check;
        }
        else if( cube_a.empty() )
        {
            return false;
        }
        else if( cube_b.empty() )
        {
            return false;
        }

        double tmp = arma::norm( arma::vectorise( cube_a ) - arma::vectorise( cube_b ), 2 );

        if( tmp > 1e-12 )
        {
            check = false;
        }

        return check;
    }

  public:

    bool operator==( const Cube& cube ) const
    {
        bool check( true );

        if( !is_equal( *this, cube ))
        {
            check = false;
        }

        // if( !is_equal( cube_index, cube.cube_index ))
        // {
        //     check = false;
        // }

        if( x_axis != cube.x_axis )
        {
            check = false;
        }

        if( y_axis != cube.y_axis )
        {
            check = false;
        }

        if( z_axis != cube.z_axis )
        {
            check = false;
        }

        return check;
    }

    bool operator!=( const Cube& cube ) const
    {
        bool check( true );

        if( is_equal( *this, cube ))
        {
            check = false;
        }

        // if( is_equal( cube_index, cube.cube_index ))
        // {
        //     check = false;
        // }

        if( x_axis == cube.x_axis )
        {
            check = false;
        }

        if( y_axis == cube.y_axis )
        {
            check = false;
        }

        if( z_axis == cube.z_axis )
        {
            check = false;
        }

        return check;
    }

    /*
     *  fill the axis name
     */
    void fill_axis_name( const std::string& x_name, const std::string& y_name, const std::string& z_name )
    {
        x_axis.set_axis( "x" );
        x_axis.set_axis_name( x_name );

        y_axis.set_axis( "y" );
        y_axis.set_axis_name( y_name );

        z_axis.set_axis( "z" );
        z_axis.set_axis_name( z_name );
    }

    Cube(
            const std::string x_name,
            const size_t x,
            const std::string y_name,
            const size_t y,
            const std::string z_name,
            const size_t z
    )
        : Base( x, y, z, arma::fill::zeros )
    {
        fill_axis_name( x_name, y_name, z_name );
    }

    /*
     *  print the axis name
     */
    template<class T>
    void print_axis_name( T& print = cpt::msg )
    {
        x_axis.print_axis( print );
        print << " : ";
        x_axis.print_axis_name( print );

        print << "\t";

        y_axis.print_axis( print );
        print << " : ";
        y_axis.print_axis_name( print );

        print << "\t";

        z_axis.print_axis( print );
        print << " : ";
        z_axis.print_axis_name( print );

        print << "\n";
    }

    /*
     *  print the axis name
     */
    template<class T>
    void print_axis_labels( T& print = cpt::msg )
    {
        x_axis.print_axis( print );
        print << " : ";
        x_axis.print_labels( print );

        print << "\n";

        y_axis.print_axis( print );
        print << " : ";
        y_axis.print_labels( print );

        print << "\n";

        z_axis.print_axis( print );
        print << " : ";
        z_axis.print_labels( print );

        print << "\n";
    }

    /*
     *  check is the axis match or not
     */
    bool check_axis_matching( const Cube& cube )
    {
        std::vector< std::string > base_name({ x_axis.get_axis_name(), y_axis.get_axis_name(), z_axis.get_axis_name() });
        std::vector< std::string > cube_name({ cube.x_axis.get_axis_name(), cube.y_axis.get_axis_name(), cube.z_axis.get_axis_name() });

        if( base_name == cube_name )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    /*
     *  re assign the axis name after grep or rotate the plane
     */
    void reassign_axis( const CubeAxis& x, const CubeAxis& y, const CubeAxis& z = CubeAxis() )
    {
        x_axis.set_axis( x.get_axis() );
        x_axis.set_axis_name( x.get_axis_name() );
        x_axis.set_labels( x.get_labels() );

        y_axis.set_axis( y.get_axis() );
        y_axis.set_axis_name( y.get_axis_name() );
        y_axis.set_labels( y.get_labels() );
        
        z_axis.set_axis( z.get_axis() );
        z_axis.set_axis_name( z.get_axis_name() );
        z_axis.set_labels( z.get_labels() );
    }

    /*
     *  fill matrix at slice by nu
     */
    void fill_nu_to_slice( const size_t& slice, const TYPE& nu )
    {
        Base::slice( slice ).fill( nu );
    }

    /*
     *  get a plane ( z, y ) by ( x )
     */
    Cube get_transversal_plane( const size_t& x )
    {
        // std::cerr << "doing get_transversal_plane: " << "X:" << z_axis.axis_name_ << " / Y:" << y_axis.axis_name_ << "\n";

        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        Cube res_cube( base_z, base_y, 1, arma::fill::zeros );

        res_cube.reassign_axis( z_axis, y_axis );

        for( size_t i = 0 ; i < base_z; ++i )
        {
            for( size_t j = 0 ; j < base_y; ++j )
            {
                res_cube( i, j, 0 ) = Base::at( x, j, i );
            }
        }

        return res_cube;
    }

    /*
     *  get a plane ( x, y ) by ( z )
     */
    Cube get_frontal_plane( const size_t& z )
    {
        // std::cerr << "doing get_frontal_plane: " << "X:" << x_axis.axis_name_ << " / Y:" << y_axis.axis_name_ << "\n";

        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;

        Cube res_cube( base_x, base_y, 1, arma::fill::zeros );
        res_cube.slice( 0 ) = Base::slice( z );

        res_cube.reassign_axis( x_axis, y_axis );

        return res_cube;
    }

    /*
     *  get a plane ( x, z ) by ( y )
     */
    Cube get_sagittal_plane( const size_t& y )
    {
        // std::cerr << "doing get_frontal_plane: " << "X:" << x_axis.axis_name_ << " / Y:" << z_axis.axis_name_ << "\n";

        size_t base_x = Base::n_rows;
        size_t base_z = Base::n_slices;

        Cube res_cube( base_x, base_z, 1, arma::fill::zeros );

        res_cube.reassign_axis( x_axis, z_axis );

        for( size_t j = 0 ; j < base_z; ++j )
        {
            for( size_t i = 0 ; i < base_x; ++i )
            {
                res_cube( i, j, 0 ) = Base::at( i, y, j );
            }
        }

        
        return res_cube;
    }

    /*
     *  inplace replace for transversal plane
     */
    void inplace_transversal_plane_replace( const size_t& x, const Mat& mat, const std::string& name )
    {
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        size_t mat_x = mat.n_rows;
        size_t mat_y = mat.n_cols;

        if( mat_x != base_z || mat_y != base_y )
        {
            throw std::runtime_error(
                ( std::string )( "Error function using in \"inplace_frontal_plane_replace\"of\"Cube< TYPE >\"\n" ) +
                ( std::string )( "It is logic error to replace:\n" ) +
                ( std::string )( "\t x " ) + std::to_string( mat_x ) + " -> " + std::to_string( base_z ) + "\n" +
                ( std::string )( "\t y " ) + std::to_string( mat_y ) + " -> " + std::to_string( base_y ) + "\n"
            );
        }

        for( size_t j = 0 ; j < mat_y; ++j )
        {
            for( size_t i = 0 ; i < mat_x; ++i )
            {
                Base::at( x, j, i ) = mat( i, j );
            }
        }

        x_axis.replace_label( x, name );
    }

    /*
     *  inplace replace for frontal plane
     */
    void inplace_frontal_plane_replace( const size_t& z, const Mat& mat, const std::string& name )
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;

        size_t mat_x = mat.n_rows;
        size_t mat_y = mat.n_cols;

        if( mat_x != base_x || mat_y != base_y )
        {
            throw std::runtime_error(
                ( std::string )( "Error function using in \"inplace_frontal_plane_replace\"of\"Cube< TYPE >\"\n" ) +
                ( std::string )( "It is logic error to replace:\n" ) +
                ( std::string )( "\t x " ) + std::to_string( mat_x ) + " -> " + std::to_string( base_x ) + "\n" +
                ( std::string )( "\t y " ) + std::to_string( mat_y ) + " -> " + std::to_string( base_y ) + "\n"
            );
        }

        for( size_t j = 0 ; j < mat_y; ++j )
        {
            for( size_t i = 0 ; i < mat_x; ++i )
            {
                Base::at( i, j, z ) = mat( i, j );
            }
        }

        z_axis.replace_label( z, name );
    }

    /*
     *  inplace replace for sagittal plane
     */
    void inplace_sagittal_plane_replace( const size_t& y, const Mat& mat, const std::string& name )
    {
        size_t base_x = Base::n_rows;
        size_t base_z = Base::n_slices;

        size_t mat_x = mat.n_rows;
        size_t mat_y = mat.n_cols;

        if( mat_x != base_x || mat_y != base_z )
        {
            throw std::runtime_error(
                ( std::string )( "Error function using in \"inplace_frontal_plane_replace\"of\"Cube< TYPE >\"\n" ) +
                ( std::string )( "It is logic error to replace:\n" ) +
                ( std::string )( "\t x " ) + std::to_string( mat_x ) + " -> " + std::to_string( base_x ) + "\n" +
                ( std::string )( "\t y " ) + std::to_string( mat_y ) + " -> " + std::to_string( base_z ) + "\n"
            );
        }

        for( size_t j = 0 ; j < mat_y; ++j )
        {
            for( size_t i = 0 ; i < mat_x; ++i )
            {
                Base::at( i, y, j ) = mat( i, j );
            }
        }

        y_axis.replace_label( y, name );

    }

    /*
     *  make cube as continuous matrix ( 1 1 1 2 2 2 )
     */
    Mat as_matrix()
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;
        size_t y = base_y * base_z;
        
        if( base_z == 1 )
        {
            return Base::slice( 0 );
        }

        Mat res_matrix( base_x, y, arma::fill::zeros );
        // Mat res_matrix( base_x, y );

        size_t cube_y = 0;
        size_t cube_z = 0;
        size_t first_col = 0;

        for( size_t k = 0; k < base_z; ++ k )
        {
            first_col = k * base_y;
            res_matrix.cols( first_col, first_col + base_y -1 ) = std::move( Base::slice( k ).cols( 0, base_y -1 ));
        }

        // for( size_t j = 0 ; j < y; ++j )
        // {
        //     for( size_t i = 0 ; i < base_x; ++i )
        //     {
        //         cube_y = j;
        //         cube_z = j / base_y; 

        //         if( j >= base_y )
        //         {
        //             cube_y = j - (( j / base_y ) * base_y );
        //         }

        //         res_matrix( i, j ) = Base::at( i, cube_y, cube_z );
        //         // std::cerr << i << "\t" << j << "\t\t\t" << i << "\t" << cube_y << "\t" << cube_z << "\n";
        //     }
        // }

        return res_matrix;
    }

    /*
     *  make continuous matrix as cube
     */
    void as_cube( const Mat& matrix, const size_t& z )
    {
        size_t mat_x = matrix.n_rows;
        size_t mat_y = matrix.n_cols;
        size_t y = mat_y / z;

        if( z == 0 )
        {
            Cube res_cube( mat_x, mat_y, z );
            // Cube res_cube( mat_x, mat_y, z, arma::fill::zeros );

            res_cube.slice( 0 ) = matrix;

            this->swap( res_cube );
            return;
        }

        Cube res_cube( mat_x, y, z );
        // Cube res_cube( mat_x, y, z, arma::fill::zeros );
        size_t first_col = 0;

        for( size_t k = 0; k < z; ++ k )
        {
            first_col = k * y;
            res_cube.slice( k ) = matrix.submat( 0, first_col, mat_x-1, first_col + y -1 );
        }

        // for( size_t j = 0 ; j < mat_y; ++j )
        // {
        //     for( size_t i = 0 ; i < mat_x; ++i )
        //     {
        //         size_t cube_y = j % y;
        //         size_t cube_z = j / y;
        //         res_cube( i, cube_y, cube_z ) = matrix.at( i, j );
        //         // std::cerr << i << "\t" << cube_y << "\t" << cube_z << "\t\t\t" << i << "\t" << j << "\n";
        //     }
        // }

        this->swap( res_cube );
    }

    /*
     *  reorder the cube to the original order after sort_by_col
     */
    void reorder_cube_by_col()
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        if( cube_index.is_empty() )
        {
            throw std::runtime_error(
                ( std::string )( "You don't need to do the \"reorder_cube\"\n" ) +
                ( std::string )( "Because this cube is still in the order\n" )
            );
        }

        Cube tmp_cube( base_x, base_y, base_z );
        // Cube tmp_cube( base_x, base_y, base_z, arma::fill::zeros );

        for( size_t i = 0; i < base_x; ++i )
        {
            for( size_t j = 0; j < base_y; ++j )
            {
                for( size_t k = 0; k < base_z; ++k )
                {
                    tmp_cube( ( size_t )( cube_index( i, j, k )), j, k ) = std::move( Base::at( i, j, k ));
                }
            }
        }

        this->Base::swap( tmp_cube );
        cube_index.clear();
    }

    /*
     *  reorder the cube to the original order after sort_by_row
     */
    void reorder_cube_by_row()
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        if( cube_index.is_empty() )
        {
            throw std::runtime_error(
                ( std::string )( "You don't need to do the \"reorder_cube\"\n" ) +
                ( std::string )( "Because this cube is still in the order\n" )
            );
        }

        Cube tmp_cube( base_x, base_y, base_z );
        // Cube tmp_cube( base_x, base_y, base_z, arma::fill::zeros );

        for( size_t i = 0; i < base_x; ++i )
        {
            for( size_t j = 0; j < base_y; ++j )
            {
                for( size_t k = 0; k < base_z; ++k )
                {
                    tmp_cube( i, ( size_t )( cube_index( i, j, k )), k ) = Base::at( i, j, k );
                }
            }
        }

        this->Base::swap( tmp_cube );
        cube_index.clear();
    }

    /*
     *  sort individual cols
     */
    void sort_by_col()
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        if( !cube_index.is_empty() )
        {
            throw std::runtime_error(
                ( std::string )( "You already have a temporary index store for the original order\n" ) +
                ( std::string )( "You may not doing individual col/row inplace sort again, this will make you lose labeling info of axes\n" ) +
                ( std::string )( "Please use \"reorder_cube\" to reorder the cube to the original order before any individual inplace sort\n" )
            );
        }

        arma::Cube< double > idx_cube( base_x, base_y, base_z );
        // arma::Cube< double > idx_cube( base_x, base_y, base_z, arma::fill::zeros );
        std::vector< TYPE > vec( base_x, 0 );

        for( size_t k = 0; k < base_z; ++k )
        {
            for( size_t j = 0; j < base_y; ++j )
            {
                // arma::Col< TYPE > col = std::move( arma::sort( Base::slice( k ).col( j ) ));
                // Base::slice( k ).col( j ) = std::move( col );
                
                vec.resize( base_x, 0 );
                
                for( int i = 0; i < base_x; ++i )
                {
                    vec[ i ] = Base::at( i, j, k );
                }

                idx_cube.slice( k ).col( j ) = std::move( arma::Col< double >( std::move( cpt::utility::sort( vec ))));
                Base::slice( k ).col( j ) = std::move( arma::Col< TYPE >( std::move( vec )));
                vec.clear();
            }
        }

        cube_index.swap( idx_cube );
    }

    /*
     *  sort individual cols, z = -1 means entire cube
     */
    Cube sort_by_col( const int& z )
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        size_t i = 0;
        size_t zi = 0;
        size_t cube_z = base_z;

        if( z != -1 )
        {
            i = z;
            base_z = z+1;
            cube_z = 1;
        }

        Cube res_cube( base_x, base_y, cube_z );
        // Cube res_cube( base_x, base_y, cube_z, arma::fill::zeros );

        for( ; i < base_z; ++i )
        {
            for( size_t j = 0; j < base_y; ++j )
            {
                arma::Col< TYPE > col( arma::sort( Base::slice( i ).col( j ) ));
                res_cube.slice( zi ).col( j ) = col;
            }

            zi++;
        }

        return res_cube;
    }

    /*
     *  sort individual rows
     */
    void sort_by_row()
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        if( !cube_index.is_empty() )
        {
            throw std::runtime_error(
                ( std::string )( "You already have a temporary index store for the original order\n" ) +
                ( std::string )( "You may not doing individual col/row inplace sort again, this will make you lose labeling info of axes\n" ) +
                ( std::string )( "Please use \"reorder_cube\" to reorder the cube to the original order before any individual inplace sort\n" )
            );
        }

        Cube< double > idx_cube( base_x, base_y, base_z );
        // Cube< double > idx_cube( base_x, base_y, base_z, arma::fill::zeros );

        for( size_t k = 0; k < base_z; ++k )
        {
            for( size_t i = 0; i < base_x; ++i )
            {
                // arma::Row< TYPE > row = std::move( arma::sort( Base::slice( k ).row( i ) ));
                // Base::slice( k ).row( i ) = std::move( row );
                
                std::vector< TYPE > vec( base_y, 0 );
                
                for( int j = 0; j < base_y; ++j )
                {
                    vec[ j ] = Base::at( i, j, k );
                }

                idx_cube.slice( k ).row( i ) = std::move( arma::Row< double >( std::move( cpt::utility::sort( vec ))));
                Base::slice( k ).row( i ) = std::move( arma::Row< TYPE >( std::move( vec )));
            }
        }

        cube_index.swap( idx_cube );
    }

    /*
     *  sort individual rows, z = -1 means entire cube
     */
    Cube sort_by_row( const int& z )
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        size_t i = 0;
        size_t zi = 0;
        size_t cube_z = base_z;

        if( z != -1 )
        {
            i = z;
            base_z = z+1;
            cube_z = 1;
        }

        Cube res_cube( base_x, base_y, cube_z );
        // Cube res_cube( base_x, base_y, cube_z, arma::fill::zeros );

        for( ; i < base_z; ++i )
        {
            for( size_t j = 0; j < base_x; ++j )
            {
                arma::Row< TYPE > row( arma::sort( Base::slice( i ).row( j ) ));
                res_cube.slice( zi ).row( j ) = row ;
            }

            zi++;
        }

        return res_cube;
    }

    /*
     *  compute the mean by col
     */
    double get_mean_by_col( const size_t& y, const size_t& z = 0 )
    {
        size_t base_x = Base::n_rows;
        TYPE total( 0 );

        arma::Col< TYPE > col( Base::slice( z ).col( y ));

        for( size_t i = 0; i < base_x; ++i )
        {
            total += col[i];
        }

        return double( total ) / double( base_x );
    }

    /*
     *  compute the mean by row
     */
    double get_mean_by_row( const size_t& x, const size_t& z = 0 )
    {
        size_t base_y = Base::n_cols;
        TYPE total( 0 );

        arma::Row< TYPE > row( Base::slice( z ).row( x ));

        for( size_t i = 0; i < base_y; ++i )
        {
            total += row[i];
        }

        return double( total ) / double( base_y );
    }

    /*
     *  compute the median by col
     */
    double get_median_by_col( const size_t& y, const size_t& z = 0 )
    {
        size_t base_x = Base::n_rows;

        arma::Col< TYPE > col( arma::sort( Base::slice( z ).col( y )));

        if( base_x % 2 == 0 )
        {
            return ( col[( base_x / 2 )- 1 ] + col[ base_x / 2 ] )/ 2;
        }
        else
        {
            return col[(( base_x - 1 )/ 2 )];
        }
    }

    /*
     *  compute the median by row
     */
    double get_median_by_row( const size_t& x, const size_t& z = 0 )
    {
        size_t base_y = Base::n_cols;

        arma::Row< TYPE > row( arma::sort( Base::slice( z ).row( x )));

        if( base_y % 2 == 0 )
        {
            return ( row[( base_y / 2 )- 1 ] + row[ base_y / 2 ] )/ 2;
        }
        else
        {
            return row[(( base_y - 1 )/ 2 )];
        }
    }

    /*
     *  inlplace dimensional rotating
     */
    void rotate_dimension( const std::vector< size_t >& order)
    {
        size_t base_x = Base::n_rows;
        size_t base_y = Base::n_cols;
        size_t base_z = Base::n_slices;

        std::vector< size_t > dims({ base_x, base_y, base_z });
        std::vector< CubeAxis > axis({ x_axis, y_axis, z_axis });

        Cube res_cube( dims[ order[0] ], dims[ order[1] ], dims[ order[2] ] );

        for( size_t k = 0; k < base_z; ++k )
        {
            dims[2] = k;
            for( size_t j = 0; j < base_y; ++j )
            {
                dims[1] = j;
                for( size_t i = 0; i < base_x; ++i )
                {
                    dims[0] = i;
                    res_cube.at( dims[ order[0] ], dims[ order[1] ], dims[ order[2] ]) = Base::at( i, j, k );
                }
            }
        }

        this->swap( res_cube );
        this->reassign_axis( axis[ order[0] ], axis[ order[1] ], axis[ order[2] ]);
    }

    /*
     *  dimensional rotating
     */
    void rotate_dimension( const Cube& cube, const std::vector< size_t >& order)
    {
        size_t base_x = cube.n_rows;
        size_t base_y = cube.n_cols;
        size_t base_z = cube.n_slices;

        std::vector< size_t > dims({ base_x, base_y, base_z });
        std::vector< CubeAxis > axis({ cube.x_axis, cube.y_axis, cube.z_axis });

        Cube res_cube( dims[ order[0] ], dims[ order[1] ], dims[ order[2] ] );

        for( size_t k = 0; k < base_z; ++k )
        {
            dims[2] = k;
            for( size_t j = 0; j < base_y; ++j )
            {
                dims[1] = j;
                for( size_t i = 0; i < base_x; ++i )
                {
                    dims[0] = i;
                    res_cube.at( dims[ order[0] ], dims[ order[1] ], dims[ order[2] ]) = cube.at( i, j, k );
                }
            }
        }

        this->swap( res_cube );
        this->reassign_axis( axis[ order[0] ], axis[ order[1] ], axis[ order[2] ]);
    }

    /*
     *  clear the Cube
     */
    void clear()
    {
        Base::clear();
        Cube blank_cube;
        this->swap( blank_cube );
    }

    /*
     *  swap the Cube
     */
    void swap( Cube& cube )
    {
        x_axis.swap( cube.x_axis );
        y_axis.swap( cube.y_axis );
        z_axis.swap( cube.z_axis );
        cube_index.swap( cube.cube_index );
        return Base::swap( cube );
    }
};

} // end of namespace format
} // end of namespace cpt
