#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/format/cube.hpp>

TEST( cube_test, copy_constructor )
{
    cpt::format::Cube< double > cube( 10, 5, 2, arma::fill::randu );

    cube.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube.x_axis.set_labels( std::vector< std::string >({ "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P9" }));
    cube.y_axis.set_labels( std::vector< std::string >({ "S0", "S1", "S2", "S3", "S4" }));
    cube.z_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));

    std::cout << "copy constructor";

    cpt::format::Cube< double > cube1( cube );

    if( cube == cube1 )
    {
        std::cout << "\rcopy constructor complete\n";
    }
    else
    {
        std::cout << "\rcopy constructor fail\n";
    }

    std::cout << "copy assignment operator";

    cpt::format::Cube< double > cube2 = cube;

    cube2.fill_axis_name( "Probesets", "Samples", "Channels" );

    if( cube == cube2 )
    {
        std::cout << "\rcopy assignment operator complete\n";
    }
    else
    {
        std::cout << "\rcopy assignment operator fail\n";
    }
}

TEST( cube_test, move_constructor )
{
    cpt::format::Cube< double > cube( 10, 5, 2, arma::fill::zeros );

    cube.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube.x_axis.set_labels( std::vector< std::string >({ "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P9" }));
    cube.y_axis.set_labels( std::vector< std::string >({ "S0", "S1", "S2", "S3", "S4" }));
    cube.z_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));

    cpt::format::Cube< double > cube1( cube );
    cpt::format::Cube< double > cube2( cube );
    cpt::format::Cube< double > cube5( cube );

    std::cout << "move constructor";

    cpt::format::Cube< double > cube3( std::move( cube1 ));

    if( cube == cube3 && cube1.empty() )
    {
        std::cout << "\rmove constructor complete\n";
    }
    else
    {
        std::cout << "\rmove constructor fail\n";
    }

    std::cout << "move assignment operator";

    cpt::format::Cube< double > cube4 = std::move( cube2 );

    if( cube == cube4 && cube2.empty() )
    {
        std::cout << "\rmove assignment operator complete\n";
    }
    else
    {
        std::cout << "\rmove assignment operator fail\n";
    }

    std::cout << "swap";

    cpt::format::Cube< double > cube6;
    cube6.swap( cube5 );

    if( cube == cube6 )
    {
        std::cout << "\rswap complete\n";
    }
    else
    {
        std::cout << "\rswap fail\n";
    }
}

TEST( cube_test, get_plane )
{
    cpt::format::Cube< double > cube( 2, 2, 2, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 1;
    cube.at( 0, 1, 0 ) = 1;
    cube.at( 1, 1, 0 ) = 1;

    cube.at( 0, 0, 1 ) = 2;
    cube.at( 1, 0, 1 ) = 2;
    cube.at( 0, 1, 1 ) = 2;
    cube.at( 1, 1, 1 ) = 2;

    cube.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube.x_axis.set_labels( std::vector< std::string >({ "P0", "P1" }));
    cube.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));
    cube.z_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));

    cpt::format::Cube< double > cube1( 2, 2, 1, arma::fill::zeros );

    cube1.at( 0, 0, 0 ) = 1;
    cube1.at( 1, 0, 0 ) = 2;
    cube1.at( 0, 1, 0 ) = 1;
    cube1.at( 1, 1, 0 ) = 2;

    cube1.fill_axis_name( "Channels", "Samples", "" );
    cube1.x_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));
    cube1.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));

    cpt::format::Cube< double > cube2( 2, 2, 1, arma::fill::zeros );

    cube2.at( 0, 0, 0 ) = 1;
    cube2.at( 1, 0, 0 ) = 1;
    cube2.at( 0, 1, 0 ) = 1;
    cube2.at( 1, 1, 0 ) = 1;

    cube2.fill_axis_name( "Probesets", "Samples", "" );
    cube2.x_axis.set_labels( std::vector< std::string >({ "P0", "P1" }));
    cube2.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));

    cpt::format::Cube< double > cube3( 2, 2, 1, arma::fill::zeros );

    cube3.at( 0, 0, 0 ) = 1;
    cube3.at( 1, 0, 0 ) = 1;
    cube3.at( 0, 1, 0 ) = 2;
    cube3.at( 1, 1, 0 ) = 2;

    cube3.fill_axis_name( "Probesets", "Channels", "" );
    cube3.x_axis.set_labels( std::vector< std::string >({ "P0", "P1" }));
    cube3.y_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));

    cpt::format::Cube< double > cube4;
    cpt::format::Cube< double > cube5;
    cpt::format::Cube< double > cube6;

    std::cout << "Get transversal plane";

    cube4 = cube.get_transversal_plane( 0 );

    if( cube1 == cube4 )
    {
        std::cout << "\rGet transversal plane complete\n";
    }
    else
    {
        std::cout << "\rGet transversal plane fail\n";
    }

    std::cout << "Get frontal plane";

    cube5 = cube.get_frontal_plane( 0 );

    if( cube2 == cube5 )
    {
        std::cout << "\rGet frontal plane complete\n";
    }
    else
    {
        std::cout << "\rGet frontal plane fail\n";
    }

    std::cout << "Get sagittal plane";

    cube6 = cube.get_sagittal_plane( 0 );

    if( cube3 == cube6 )
    {
        std::cout << "\rGet sagittal plane complete\n";
    }
    else
    {
        std::cout << "\rGet sagittal plane fail\n";
    }
}

TEST( cube_test, inplace_plane_replace )
{
    cpt::format::Cube< double > cube( 2, 2, 2, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 1;
    cube.at( 0, 1, 0 ) = 1;
    cube.at( 1, 1, 0 ) = 1;

    cube.at( 0, 0, 1 ) = 2;
    cube.at( 1, 0, 1 ) = 2;
    cube.at( 0, 1, 1 ) = 2;
    cube.at( 1, 1, 1 ) = 2;

    cube.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube.x_axis.set_labels( std::vector< std::string >({ "P0", "P1" }));
    cube.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));
    cube.z_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));

    cpt::format::Cube< double > cube1( 2, 2, 2, arma::fill::zeros );

    cube1.at( 0, 0, 0 ) = 0;
    cube1.at( 1, 0, 0 ) = 1;
    cube1.at( 0, 1, 0 ) = 0;
    cube1.at( 1, 1, 0 ) = 1;

    cube1.at( 0, 0, 1 ) = 0;
    cube1.at( 1, 0, 1 ) = 2;
    cube1.at( 0, 1, 1 ) = 0;
    cube1.at( 1, 1, 1 ) = 2;

    cube1.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube1.x_axis.set_labels( std::vector< std::string >({ "R0", "P1" }));
    cube1.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));
    cube1.z_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));

    cpt::format::Cube< double > cube2( 2, 2, 2, arma::fill::zeros );

    cube2.at( 0, 0, 0 ) = 0;
    cube2.at( 1, 0, 0 ) = 0;
    cube2.at( 0, 1, 0 ) = 0;
    cube2.at( 1, 1, 0 ) = 0;

    cube2.at( 0, 0, 1 ) = 2;
    cube2.at( 1, 0, 1 ) = 2;
    cube2.at( 0, 1, 1 ) = 2;
    cube2.at( 1, 1, 1 ) = 2;

    cube2.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube2.x_axis.set_labels( std::vector< std::string >({ "P0", "P1" }));
    cube2.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));
    cube2.z_axis.set_labels( std::vector< std::string >({ "R0", "C1" }));

    cpt::format::Cube< double > cube3( 2, 2, 2, arma::fill::zeros );

    cube3.at( 0, 0, 0 ) = 0;
    cube3.at( 1, 0, 0 ) = 0;
    cube3.at( 0, 1, 0 ) = 1;
    cube3.at( 1, 1, 0 ) = 1;

    cube3.at( 0, 0, 1 ) = 0;
    cube3.at( 1, 0, 1 ) = 0;
    cube3.at( 0, 1, 1 ) = 2;
    cube3.at( 1, 1, 1 ) = 2;

    cube3.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube3.x_axis.set_labels( std::vector< std::string >({ "P0", "P1" }));
    cube3.y_axis.set_labels( std::vector< std::string >({ "R0", "S1" }));
    cube3.z_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));

    cpt::format::Cube< double > cube4( cube );
    cpt::format::Cube< double > cube5( cube );
    cpt::format::Cube< double > cube6( cube );
    cpt::format::Cube< double > cube7;

    std::cout << "Inplace transversal plane replace";

    cube7 = cube.get_transversal_plane( 0 );
    cube7.slice( 0 ) = arma::Mat< double >( cube7.n_rows, cube7.n_cols, arma::fill::zeros );
    cube4.inplace_transversal_plane_replace( 0, cube7.as_matrix(), "R0" );

    if( cube1 == cube4 )
    {
        std::cout << "\rInplace transversal plane replace complete\n";
    }
    else
    {
        std::cout << "\rInplace transversal plane replace fail\n";
    }

    std::cout << "Inplace frontal plane replace";

    cube7 = cube.get_frontal_plane( 0 );
    cube7.slice( 0 ) = arma::Mat< double >( cube7.n_rows, cube7.n_cols, arma::fill::zeros );
    cube5.inplace_frontal_plane_replace( 0, cube7.as_matrix(), "R0" );

    if( cube2 == cube5 )
    {
        std::cout << "\rInplace frontal plane replace complete\n";
    }
    else
    {
        std::cout << "\rInplace frontal plane replace fail\n";
    }

    std::cout << "Inplace sagittal plane replace";

    cube7 = cube.get_sagittal_plane( 0 );
    cube7.slice( 0 ) = arma::Mat< double >( cube7.n_rows, cube7.n_cols, arma::fill::zeros );
    cube6.inplace_sagittal_plane_replace( 0, cube7.as_matrix(), "R0" );

    if( cube3 == cube6 )
    {
        std::cout << "\rInplace sagittal plane replace complete\n";
    }
    else
    {
        std::cout << "\rInplace sagittal plane replace fail\n";
    }
}

TEST( cube_test, as_matrix_as_cube )
{
    cpt::format::Cube< double > cube( 2, 2, 2, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 1;
    cube.at( 0, 1, 0 ) = 1;
    cube.at( 1, 1, 0 ) = 1;

    cube.at( 0, 0, 1 ) = 2;
    cube.at( 1, 0, 1 ) = 2;
    cube.at( 0, 1, 1 ) = 2;
    cube.at( 1, 1, 1 ) = 2;

    cpt::format::Cube< double > cube1;

    std::cout << "As continuous matrix as cube";

    cube1.as_cube( cube.as_matrix(), 2 );

    if( cube == cube1 )
    {
        std::cout << "\rAs continuous matrix as cube complete\n";
    }
    else
    {
        std::cout << "\rAs continuous matrix as cube fail\n";
    }

}

TEST( cube_test, as_matrix_as_cube2 )
{
    cpt::format::Cube< double > cube( 2, 1, 2, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 1;

    cube.at( 0, 0, 1 ) = 2;
    cube.at( 1, 0, 1 ) = 2;

    cpt::format::Cube< double > cube1;

    std::cout << "As continuous matrix as cube";

    cube1.as_cube( cube.as_matrix(), 2 );

    if( cube == cube1 )
    {
        std::cout << "\rAs continuous matrix as cube complete\n";
    }
    else
    {
        std::cout << "\rAs continuous matrix as cube fail\n";
    }

}

TEST( cube_test, as_matrix_as_cube3 )
{
    cpt::format::Cube< double > cube( 2, 2, 3, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 1;
    cube.at( 0, 1, 0 ) = 1;
    cube.at( 1, 1, 0 ) = 1;

    cube.at( 0, 0, 1 ) = 2;
    cube.at( 1, 0, 1 ) = 2;
    cube.at( 0, 1, 1 ) = 2;
    cube.at( 1, 1, 1 ) = 2;

    cube.at( 0, 0, 2 ) = 3;
    cube.at( 1, 0, 2 ) = 3;
    cube.at( 0, 1, 2 ) = 3;
    cube.at( 1, 1, 2 ) = 3;

    cpt::format::Cube< double > cube1;

    std::cout << "As continuous matrix as cube";

    cube1.as_cube( cube.as_matrix(), 2 );

    if( cube == cube1 )
    {
        std::cout << "\rAs continuous matrix as cube complete\n";
    }
    else
    {
        std::cout << "\rAs continuous matrix as cube fail\n";
    }

}

TEST( cube_test, get_sort )
{
    cpt::format::Cube< double > cube( 3, 3, 1, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 3;
    cube.at( 2, 0, 0 ) = 2;

    cube.at( 0, 1, 0 ) = 3;
    cube.at( 1, 1, 0 ) = 2;
    cube.at( 2, 1, 0 ) = 1;

    cube.at( 0, 2, 0 ) = 2;
    cube.at( 1, 2, 0 ) = 1;
    cube.at( 2, 2, 0 ) = 3;

    cpt::format::Cube< double > cube1( 3, 3, 1, arma::fill::zeros );

    cube1.at( 0, 0, 0 ) = 1;
    cube1.at( 1, 0, 0 ) = 2;
    cube1.at( 2, 0, 0 ) = 3;

    cube1.at( 0, 1, 0 ) = 1;
    cube1.at( 1, 1, 0 ) = 2;
    cube1.at( 2, 1, 0 ) = 3;

    cube1.at( 0, 2, 0 ) = 1;
    cube1.at( 1, 2, 0 ) = 2;
    cube1.at( 2, 2, 0 ) = 3;

    cpt::format::Cube< double > cube2( 3, 3, 1, arma::fill::zeros );

    cube2.at( 0, 0, 0 ) = 1;
    cube2.at( 1, 0, 0 ) = 1;
    cube2.at( 2, 0, 0 ) = 1;

    cube2.at( 0, 1, 0 ) = 2;
    cube2.at( 1, 1, 0 ) = 2;
    cube2.at( 2, 1, 0 ) = 2;

    cube2.at( 0, 2, 0 ) = 3;
    cube2.at( 1, 2, 0 ) = 3;
    cube2.at( 2, 2, 0 ) = 3;

    cpt::format::Cube< double > cube3;
    cpt::format::Cube< double > cube4;

    std::cout << "Sorting by col";

    cube3 = cube.sort_by_col( 0 );

    if( cube3 == cube1 )
    {
        std::cout << "\rSorting by col complete\n";
    }
    else
    {
        std::cout << "\rSorting by col fail\n";
    }

    std::cout << "Sorting by row";

    cube4 = cube.sort_by_row( 0 );

    if( cube4 == cube2 )
    {
        std::cout << "\rSorting by row complete\n";
    }
    else
    {
        std::cout << "\rSorting by row fail\n";
    }
}

TEST( cube_test, inplace_sort )
{
    cpt::format::Cube< double > cube( 3, 3, 1, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 3;
    cube.at( 2, 0, 0 ) = 2;

    cube.at( 0, 1, 0 ) = 3;
    cube.at( 1, 1, 0 ) = 2;
    cube.at( 2, 1, 0 ) = 1;

    cube.at( 0, 2, 0 ) = 2;
    cube.at( 1, 2, 0 ) = 1;
    cube.at( 2, 2, 0 ) = 3;

    cpt::format::Cube< double > cube1( 3, 3, 1, arma::fill::zeros );

    cube1.at( 0, 0, 0 ) = 1;
    cube1.at( 1, 0, 0 ) = 2;
    cube1.at( 2, 0, 0 ) = 3;

    cube1.at( 0, 1, 0 ) = 1;
    cube1.at( 1, 1, 0 ) = 2;
    cube1.at( 2, 1, 0 ) = 3;

    cube1.at( 0, 2, 0 ) = 1;
    cube1.at( 1, 2, 0 ) = 2;
    cube1.at( 2, 2, 0 ) = 3;

    cpt::format::Cube< double > cube2( 3, 3, 1, arma::fill::zeros );

    cube2.at( 0, 0, 0 ) = 1;
    cube2.at( 1, 0, 0 ) = 1;
    cube2.at( 2, 0, 0 ) = 1;

    cube2.at( 0, 1, 0 ) = 2;
    cube2.at( 1, 1, 0 ) = 2;
    cube2.at( 2, 1, 0 ) = 2;

    cube2.at( 0, 2, 0 ) = 3;
    cube2.at( 1, 2, 0 ) = 3;
    cube2.at( 2, 2, 0 ) = 3;

    cpt::format::Cube< double > cube3( cube );
    cpt::format::Cube< double > cube4( cube );

    std::cout << "Inplace sort by col";

    cube3.sort_by_col();

    if( cube3 == cube1 )
    {
        std::cout << "\rSorting by col complete\n";

        std::cout << "Inplace reorder by col";

        cube3.reorder_cube_by_col();

        if( cube3 == cube )
        {
            std::cout << "\rInplace reorder by col complete\n";
        }
        else
        {
            std::cout << "\rInplace reorder by col fail\n";
        }
    }
    else
    {
        std::cout << "\rSorting by col fail\n";
    }
    
    std::cout << "Inplace sort by row";

    cube4.sort_by_row();

    if( cube4 == cube2 )
    {
        std::cout << "\rSorting by row complete\n";

        std::cout << "Inplace reorder by row";

        cube4.reorder_cube_by_row();

        if( cube4 == cube )
        {
            std::cout << "\rInplace reorder by row complete\n";
        }
        else
        {
            std::cout << "\rInplace reorder by row fail\n";
        }
    }
    else
    {
        std::cout << "\rSorting by row fail\n";
    }
}

TEST( cube_test, get_mean_median )
{
    cpt::format::Cube< double > cube( 2, 3, 1, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 6;

    cube.at( 0, 1, 0 ) = 3;
    cube.at( 1, 1, 0 ) = 5;

    cube.at( 0, 2, 0 ) = 2;
    cube.at( 1, 2, 0 ) = 4;

    std::cout << "Get mean by col";

    if( cube.get_mean_by_col( 1 ) == 4 )
    {
        std::cout << "\rGet mean by col complete\n";
    }
    else
    {
        std::cout << "\rGet mean by col fail\n";
    }

    std::cout << "Get mean by row";

    if( cube.get_mean_by_row( 0 ) == 2 )
    {
        std::cout << "\rGet mean by row complete\n";
    }
    else
    {
        std::cout << "\rGet mean by row fail\n";
    }

    std::cout << "Get median by col";

    if( cube.get_median_by_col( 1 ) == 4 )
    {
        std::cout << "\rGet median by col complete\n";
    }
    else
    {
        std::cout << "\rGet median by col fail\n";
    }

    std::cout << "Get median by row";

    if( cube.get_median_by_row( 0 ) == 2 )
    {
        std::cout << "\rGet median by row complete\n";
    }
    else
    {
        std::cout << "\rGet median by row fail\n";
    }
}

TEST( cube_test, rotate )
{
    cpt::format::Cube< double > cube( 3, 2, 1, arma::fill::zeros );

    cube.at( 0, 0, 0 ) = 1;
    cube.at( 1, 0, 0 ) = 2;
    cube.at( 2, 0, 0 ) = 3;
    cube.at( 0, 1, 0 ) = 4;
    cube.at( 1, 1, 0 ) = 5;
    cube.at( 2, 1, 0 ) = 6;

    cube.fill_axis_name( "Probesets", "Samples", "Channels" );
    cube.x_axis.set_labels( std::vector< std::string >({ "P0", "P1", "P3" }));
    cube.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));
    cube.z_axis.set_labels( std::vector< std::string >({ "C0" }));

    cpt::format::Cube< double > cube1( 1, 2, 3, arma::fill::zeros );

    cube1.at( 0, 0, 0 ) = 1;
    cube1.at( 0, 0, 1 ) = 2;
    cube1.at( 0, 0, 2 ) = 3;
    cube1.at( 0, 1, 0 ) = 4;
    cube1.at( 0, 1, 1 ) = 5;
    cube1.at( 0, 1, 2 ) = 6;

    cube1.fill_axis_name( "Channels", "Samples", "Probesets" );
    cube1.z_axis.set_labels( std::vector< std::string >({ "C0" }));
    cube1.y_axis.set_labels( std::vector< std::string >({ "S0", "S1" }));
    cube1.x_axis.set_labels( std::vector< std::string >({ "P0", "P1", "P3" }));

    cpt::format::Cube< double > cube2;

    std::cout << "Get dimension rotate";

    cube2.rotate_dimension( cube, std::vector< size_t >({ 2, 1, 0 }));

    if( cube2 == cube1 )
    {
        std::cout << "\rGet dimension rotate complete\n";
    }
    else
    {
        std::cout << "\rGet dimension rotate fail\n";
    }

    std::cout << "Inplace dimension rotate";

    cube2.rotate_dimension( std::vector< size_t >({ 2, 1, 0 }));

    if( cube2 == cube )
    {
        std::cout << "\rInplace dimension rotate complete\n";
    }
    else
    {
        std::cout << "\rInplace dimension rotate fail\n";
    }
}
