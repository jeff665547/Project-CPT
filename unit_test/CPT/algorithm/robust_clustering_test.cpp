#include <iostream>
#include <ctime>
#include <chrono>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <Nucleona/app/cli/gtest.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <CPT/algorithm/robust_clustering.hpp>

void output_vec( std::vector< double >& vec )
{
    std::cout << std::endl << "Input Vec:" << std::endl;

    for( auto& num : vec )
        std::cout << " " << num;

    std::cout << std::endl;
}

void output_bdy( std::pair< double, cpt::algorithm::RobustClusteringUtility >& ml, std::vector< double >& vec )
{
    std::cout
        << std::endl
        << "Maximum Likelihood: " << ml.first
        << "\tMaximum Likelihood per DataPoint: " << ml.first / vec.size();

    std::cout << "\tBoundary: 0";

    for( auto& boundary : ml.second.boundarys )
    {
        std::cout << "-" << "\033[1;31m(" << boundary << ")\033[0m";
    }

    std::cout << "-" << vec.size() << std::endl;
}

void output_res( std::pair< double, cpt::algorithm::RobustClusteringUtility >& ml, std::vector< double >& vec )
{
    std::cout
        << std::endl
        << "Maximum Likelihood: " << std::get< 0 >( ml )
        << "\tMaximum Likelihood per DataPoint: " << std::get< 0 >( ml ) / vec.size();

    std::cout << "\tBoundary: 0";

    for( auto& boundary : ml.second.boundarys )
    {
        std::cout << "-" << "\033[1;31m(" << boundary << ")\033[0m";
    }

    std::cout << "-" << vec.size() << std::endl;

    size_t idx( 0 );

    for( size_t i = 0; i < vec.size(); ++i )
    {
        if( i != 0 && i != vec.size() )
            std::cout << " ";

        if( i == ml.second.boundarys[ idx ] )
        {
            std::cout << "\033[1;31m| \033[0m";
            ++idx;
        }

        std::cout << vec[ i ];
    }

    std::cout << std::endl;
}

std::chrono::time_point< std::chrono::system_clock > get_time_point()
{
    std::chrono::time_point< std::chrono::system_clock > time;
    time = std::chrono::system_clock::now();
    return time;
}

std::vector< std::pair< double, size_t >> get_file( const std::string& file_name )
{
    std::vector< std::pair< double, size_t >> res;

	std::string line;
	std::ifstream file( file_name );

	while( std::getline( file, line ) )
    {
        std::vector< std::string > vec;
        boost::iter_split( vec, line, boost::algorithm::first_finder( "  " ));
        res.emplace_back( std::make_pair( std::stod( vec[1] ), std::stoull( vec[2] )));
    }

    return res;
}

std::vector< double > to_data_vector( const std::vector< std::pair< double, size_t >>& pair_vec )
{
    std::vector< double > vec;

    for( auto& pair : pair_vec )
    {
        vec.emplace_back( pair.first );
    }

    std::sort( vec.begin(), vec.end() );

    return vec;
}

void count(
        std::chrono::time_point< std::chrono::system_clock >& st,
        std::chrono::time_point< std::chrono::system_clock >& et
        )
{
    std::chrono::duration< double > est = et - st;
    double t = est.count();
    std::string str = std::to_string( t );
    std::cout << std::endl << "Time Cost: " << str << " second" << std::endl << std::endl;
}

/*
TEST( robust_clustering_test, boundary_mean )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    std::cout << "boundary mean";
    mlc.boundary_mean_estimate( 0, 0, vec.size() );

    if( mlc.utility.means[ 0 ] == 5.5 )
    {
        std::cout << "\rboundary mean complete";
    }
    else
    {
        std::cout << "\rboundary mean fail";
    }

    std::cout << std::endl;
}

TEST( robust_clustering_test, incremental_mean )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    std::cout << "incremental mean";
    mlc.boundary_mean_estimate( 0, 0, vec.size()-1 );

    mlc.last_means[ 0 ] = mlc.utility.means[ 0 ];
    mlc.incremental_mean_estimate( 0, 10, 10 );

    if( mlc.utility.means[ 0 ] == 5.5 )
    {
        std::cout << "\rincremental mean complete";
    }
    else
    {
        std::cout << "\rincremental mean fail";
    }

    std::cout << std::endl;
}

TEST( robust_clustering_test, decremental_mean )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    std::cout << "decremental mean";
    mlc.boundary_mean_estimate( 0, 0, vec.size() );

    mlc.last_means[ 0 ] = mlc.utility.means[ 0 ];
    mlc.decremental_mean_estimate( 0, 10, 10 );

    if( mlc.utility.means[ 0 ] == 5 )
    {
        std::cout << "\rdecremental mean complete";
    }
    else
    {
        std::cout << "\rdecremental mean fail";
    }

    std::cout << std::endl;
}

TEST( robust_clustering_test, boundary_variances )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    std::cout << "boundary variance";
    mlc.boundary_variance_estimate( 0, 0, vec.size() );

    if( mlc.utility.variances[ 0 ] >= 9.16666 && mlc.utility.variances[ 0 ] <= 9.16667 )
    {
        std::cout << "\rboundary variance complete";
    }
    else
    {
        std::cout << "\rboundary variance fail";
    }

    std::cout << std::endl;
}

TEST( robust_clustering_test, incremental_variances )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    std::cout << "incremental variance";
    mlc.boundary_variance_estimate( 0, 0, vec.size()-1 );

    mlc.boundary_mean_estimate( 0, 0, vec.size()-1 );
    mlc.last_means[ 0 ] = mlc.utility.means[ 0 ];

    mlc.last_variances[ 0 ] = mlc.utility.variances[ 0 ];
    mlc.incremental_variance_estimate( 0, 10, 10 );

    if( mlc.utility.variances[ 0 ] >= 9.16666 && mlc.utility.variances[ 0 ] <= 9.16667 )
    {
        std::cout << "\rincremental variance complete";
    }
    else
    {
        std::cout << "\rincremental variance fail";
    }

    std::cout << std::endl;
}

TEST( robust_clustering_test, decremental_variances )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    std::cout << "decremental variance";
    mlc.boundary_variance_estimate( 0, 0, vec.size() );

    mlc.boundary_mean_estimate( 0, 0, vec.size()-1 );
    mlc.last_means[ 0 ] = mlc.utility.means[ 0 ];

    mlc.last_variances[ 0 ] = mlc.utility.variances[ 0 ];
    mlc.decremental_variance_estimate( 0, 10, 10 );
    
    if( mlc.utility.variances[ 0 ] >= 7.44444 &&  mlc.utility.variances[ 0 ] <= 7.55555 )
    {
        std::cout << "\rdecremental variance complete";
    }
    else
    {
        std::cout << "\rdecremental variance fail";
    }

    std::cout << std::endl;
}

TEST( robust_clustering_test, sweeping )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    std::cout << "sweeping";
    std::vector< cpt::algorithm::RobustClusteringUtility > utilities;

    while( true )
    {
        utilities.emplace_back( mlc.utility );
        mlc.last_boundarys = mlc.utility.boundarys;           

        if( mlc.is_border( mlc.utility.boundarys[0] ))
        {
            break;
        }

        if( mlc.is_border( mlc.utility.boundarys[ mlc.utility.boundarys.size() -1 ] ))
        {
            mlc.move_boundarys();
        }
        else
        {
            mlc.utility.boundarys[ mlc.utility.boundarys.size() -1 ]++;
        }
    }

    // for( auto& utility : utilities )
    // {
    //     std::cout << "Boundary:";

    //     for( auto& boundary : utility.boundarys )
    //     {
    //         std::cout << " " << boundary;
    //     }

    //     std::cout << std::endl;
    // }

    std::cout << "\rsweeping complete" << std::endl;
}

TEST( robust_clustering_test, basic_testing )
{
    std::vector< double > vec{ -2.05, -2, -1.95, -0.05, 0, 0.05, 1.95, 2, 2.05 };
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    auto stime = get_time_point();
    auto ml = mlc.run();
    auto etime = get_time_point();

    // output_vec( vec );
    // output_res( ml, vec );
    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( robust_clustering_test, case_1 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_1.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    auto stime = get_time_point();
    auto ml = mlc.run();
    auto etime = get_time_point();

    // output_vec( vec );
    // output_res( ml, vec );
    output_bdy( ml, vec );
    count( stime, etime );
}
*/

TEST( robust_clustering_test, case_2 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_2.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    auto stime = get_time_point();
    auto ml = mlc.run();
    auto etime = get_time_point();

    // output_vec( vec );
    // output_res( ml, vec );
    output_bdy( ml, vec );
    count( stime, etime );
}

/*
TEST( robust_clustering_test, case_3 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_3.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    auto stime = get_time_point();
    auto ml = mlc.run();
    auto etime = get_time_point();

    // output_vec( vec );
    // output_res( ml, vec );
    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( robust_clustering_test, case_4 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_4.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    auto stime = get_time_point();
    auto ml = mlc.run();
    auto etime = get_time_point();

    // output_vec( vec );
    // output_res( ml, vec );
    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( robust_clustering_test, case_5 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_5.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::RobustClustering< double > mlc( vec, 3 );

    auto stime = get_time_point();
    auto ml = mlc.run();
    auto etime = get_time_point();

    // output_vec( vec );
    // output_res( ml, vec );
    output_bdy( ml, vec );
    count( stime, etime );
}
*/
