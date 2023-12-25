#include <Nucleona/app/cli/gtest.hpp>
#include <iostream>
#include <CPT/utility/sort.hpp>
#include <CPT/format/cube.hpp>

TEST( sort_test, sort_object_and_return_original_index )
{
    std::vector< size_t > vec;

    vec.push_back( 9 );
    vec.push_back( 2 );
    vec.push_back( 8 );
    vec.push_back( 1 );
    vec.push_back( 4 );
    vec.push_back( 7 );
    vec.push_back( 6 );
    vec.push_back( 0 );
    vec.push_back( 3 );
    vec.push_back( 5 );

    std::cout << "un-sort:";

    for( auto& val : vec )
    {
        std::cout << "\t" << val;
    }

    std::cout << "\n";

    std::vector< size_t > idx = cpt::utility::sort( vec );

    std::cout << "sorted :";

    for( auto& val : vec )
    {
        std::cout << "\t" << val;
    }

    std::cout << "\n";

    std::cout << "index  :";

    for( auto& val : idx )
    {
        std::cout << "\t" << val;
    }

    std::cout << "\n";
}

TEST( sort_test, sort_object_and_return_original_index_test )
{
    size_t x = 20;
    size_t y = 1;
    size_t z = 1;

    cpt::format::Cube< double > cube( x, y, z );
    cube.slice( 0 ).col( 0 ) = arma::Col< double >( std::vector< double >({ 66,  4, 85, 94, 68, 76, 75, 40, 66, 18, 71,  4, 28,  5, 10, 83, 70, 32, 96,  4 }));
    cube.print();

    cube.sort_by_col();
    cube.print();

    cube.reorder_cube_by_col();
    cube.print();
}
