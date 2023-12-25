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
#include <CPT/algorithm/brlmmp.hpp>

void output_bdy( std::pair< double, cpt::algorithm::BRLMMpBufferType >& buffer, std::vector< double >& vec )
{
    std::cout
        << std::endl
        << "Maximum Likelihood: " << buffer.first
        << "\tMaximum Likelihood per DataPoint: " << buffer.first / vec.size();

    std::cout << "\tBoundary: 0";

    for( auto& boundary : buffer.second.boundarys )
    {
        std::cout << "-" << "\033[1;31m(" << boundary << ")\033[0m";
    }

    std::cout << "-" << vec.size();

    std::cout << "\tmeans:";
    
    for( auto& mean : buffer.second.means )
    {
        std::cout << " " << mean;
    }

    std::cout << "\tvariances:";
    
    for( auto& std : buffer.second.stds )
    {
        std::cout << " " << std;
    }
}

void output_bic( cpt::algorithm::BRLMMpBufferType& buffer, std::vector< double >& vec )
{
    std::cout << "\nBoundary: 0";

    for( auto& boundary : buffer.boundarys )
    {
        std::cout << "-" << "\033[1;31m(" << boundary << ")\033[0m";
    }

    std::cout << "-" << vec.size();

    std::cout << "\tmeans:";
    
    for( auto& mean : buffer.means )
    {
        std::cout << " " << mean;
    }
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
TEST( brlmmp_test, boundary_mean )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    std::cout << "boundary mean";
    bc.boundary_mean_estimate( 0, 0, vec.size(), bc.buffer.means, vec );

    if( bc.buffer.means[ 0 ] == 5.5 )
    {
        std::cout << "\rboundary mean complete";
    }
    else
    {
        std::cout << "\rboundary mean fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, incremental_mean )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    std::cout << "incremental mean";
    bc.boundary_mean_estimate( 0, 0, vec.size()-1, bc.buffer.means, vec );

    bc.last_means[ 0 ] = bc.buffer.means[ 0 ];
    bc.incremental_mean_estimate( 0, 10, 10, bc.buffer.means );

    if( bc.buffer.means[ 0 ] == 5.5 )
    {
        std::cout << "\rincremental mean complete";
    }
    else
    {
        std::cout << "\rincremental mean fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, decremental_mean )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    std::cout << "decremental mean";
    bc.boundary_mean_estimate( 0, 0, vec.size(), bc.buffer.means, vec );

    bc.last_means[ 0 ] = bc.buffer.means[ 0 ];
    bc.decremental_mean_estimate( 0, 10, 10, bc.buffer.means );

    if( bc.buffer.means[ 0 ] == 5 )
    {
        std::cout << "\rdecremental mean complete";
    }
    else
    {
        std::cout << "\rdecremental mean fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, boundary_variances )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    std::cout << "boundary variance";
    bc.boundary_variance_estimate( 0, 0, vec.size(), bc.buffer.variances, vec );

    if( bc.buffer.variances[ 0 ] >= 9.16666 && bc.buffer.variances[ 0 ] <= 9.16667 )
    {
        std::cout << "\rboundary variance complete";
    }
    else
    {
        std::cout << "\rboundary variance fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, incremental_variances )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    std::cout << "incremental variance";
    bc.boundary_variance_estimate( 0, 0, vec.size()-1, bc.buffer.variances, vec );

    bc.boundary_mean_estimate( 0, 0, vec.size()-1, bc.buffer.means, vec );
    bc.last_means[ 0 ] = bc.buffer.means[ 0 ];

    bc.last_variances[ 0 ] = bc.buffer.variances[ 0 ];
    bc.incremental_variance_estimate( 0, 10, 10, bc.buffer.variances );

    if( bc.buffer.variances[ 0 ] >= 9.16666 && bc.buffer.variances[ 0 ] <= 9.16667 )
    {
        std::cout << "\rincremental variance complete";
    }
    else
    {
        std::cout << "\rincremental variance fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, decremental_variances )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    std::cout << "decremental variance";
    bc.boundary_variance_estimate( 0, 0, vec.size(), bc.buffer.variances, vec );

    bc.boundary_mean_estimate( 0, 0, vec.size()-1, bc.buffer.means, vec );
    bc.last_means[ 0 ] = bc.buffer.means[ 0 ];

    bc.last_variances[ 0 ] = bc.buffer.variances[ 0 ];
    bc.decremental_variance_estimate( 0, 10, 10, bc.buffer.variances, bc.buffer.means );
    
    if( bc.buffer.variances[ 0 ] >= 7.44444 &&  bc.buffer.variances[ 0 ] <= 7.55555 )
    {
        std::cout << "\rdecremental variance complete";
    }
    else
    {
        std::cout << "\rdecremental variance fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, sweeping )
{
    std::vector< double > vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    std::cout << "sweeping";
    std::vector< cpt::algorithm::BRLMMpBufferType > buffers;

    while( true )
    {
        buffers.emplace_back( bc.buffer );
        bc.last_boundarys = bc.buffer.boundarys;           

        if( bc.is_border( bc.buffer.boundarys[0], vec ))
        {
            break;
        }

        if( bc.is_border( bc.buffer.boundarys[ bc.buffer.boundarys.size() -1 ], vec ))
        {
            bc.boundarys_rearrange( bc.buffer.boundarys, vec );
        }
        else
        {
            bc.buffer.boundarys[ bc.buffer.boundarys.size() -1 ]++;
        }
    }

    std::cout << "\rsweeping complete" << std::endl;
}
*/

TEST( brlmmp_test, clustering_basic_testing )
{
    std::vector< double > vec{ -2.05, -2, -1.95, -0.05, 0, 0.05, 1.95, 2, 2.05 };
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    auto stime = get_time_point();
    auto ml = bc.run();
    auto etime = get_time_point();

    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_case_1 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_1.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    auto stime = get_time_point();
    auto ml = bc.run();
    auto etime = get_time_point();

    output_bdy( ml, vec );
    count( stime, etime );
}

/*
TEST( brlmmp_test, clustering_case_2 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_2.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    auto stime = get_time_point();
    auto ml = bc.run();
    auto etime = get_time_point();

    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_case_3 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_3.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    auto stime = get_time_point();
    auto ml = bc.run();
    auto etime = get_time_point();

    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_case_4 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_4.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    auto stime = get_time_point();
    auto ml = bc.run();
    auto etime = get_time_point();

    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_case_5 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_5.txt" ));

    std::vector< double > vec( to_data_vector( datas ));
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    auto stime = get_time_point();
    auto ml = bc.run();
    auto etime = get_time_point();

    output_bdy( ml, vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_real_case_1 )
{
    std::vector< double > vec{
          -0.785981
        , -0.0281127
        , -0.011424
        , -0.00901137
        , -0.00585408
        ,  0.0291635
        ,  0.10524
        ,  0.190046
        ,  0.202006
        ,  0.215867
        ,  0.826461
        ,  0.829882
        ,  0.844959
        ,  0.847172
        ,  0.847438
        ,  0.849716
        ,  0.852221
        ,  0.853378
        ,  0.855153
        ,  0.855799
        ,  0.856266
        ,  0.85744 
        ,  0.861222
        ,  0.866559
        ,  0.870079
        ,  0.87342 
        ,  0.876927
        ,  0.878194
        ,  0.879081
        ,  0.883319
    };

    std::vector< double > likelihoods;

    for( size_t i = 1; i <= 3; ++i )
    {
        cpt::algorithm::BRLMMpClustering bc( vec, i );

        auto stime = get_time_point();
        auto ml = bc.run();
        auto etime = get_time_point();

        likelihoods.emplace_back( ml.first );
        output_bdy( ml, vec );
        count( stime, etime );
    }

    std::cout << "MinBic";

    for( size_t k = likelihoods.size(); k != 0; --k )
    {
        double bic = -2 - likelihoods[ k-1 ] + ( 3 * k - 1 ) * log( vec.size() );
        std::cout << "\t" << k << ":" << bic;
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, find_MinBIC_real_case_1 )
{
    std::vector< double > vec{
          -0.785981
        , -0.0281127
        , -0.011424
        , -0.00901137
        , -0.00585408
        ,  0.0291635
        ,  0.10524
        ,  0.190046
        ,  0.202006
        ,  0.215867
        ,  0.826461
        ,  0.829882
        ,  0.844959
        ,  0.847172
        ,  0.847438
        ,  0.849716
        ,  0.852221
        ,  0.853378
        ,  0.855153
        ,  0.855799
        ,  0.856266
        ,  0.85744 
        ,  0.861222
        ,  0.866559
        ,  0.870079
        ,  0.87342 
        ,  0.876927
        ,  0.878194
        ,  0.879081
        ,  0.883319
    };

    cpt::algorithm::BRLMMp brlmmp( 3 );

    auto stime = get_time_point();
    brlmmp.find_min_bic( vec );
    auto etime = get_time_point();

    output_bic( brlmmp.buffers[0], vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_real_case_2 )
{
    std::vector< double > vec{
          -0.121113
        , -0.11675
        ,  0.0685741
        ,  0.0972767
        ,  0.164565
        ,  0.227398
        ,  0.237715
        ,  0.851805
        ,  0.868675
        ,  0.885955
    };

    std::vector< double > likelihoods;

    for( size_t i = 1; i <= 3; ++i )
    {
        cpt::algorithm::BRLMMpClustering bc( vec, i );

        auto stime = get_time_point();
        auto ml = bc.run();
        auto etime = get_time_point();

        likelihoods.emplace_back( ml.first );
        output_bdy( ml, vec );
        count( stime, etime );
    }

    std::cout << "MinBic";

    for( size_t k = likelihoods.size(); k != 0; --k )
    {
        double bic = -2 - likelihoods[ k-1 ] + ( 3 * k - 1 ) * log( vec.size() );
        std::cout << "\t" << k << ":" << bic;
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, find_MinBIC_real_case_2 )
{
    std::vector< double > vec{
          -0.121113
        , -0.11675
        ,  0.0685741
        ,  0.0972767
        ,  0.164565
        ,  0.227398
        ,  0.237715
        ,  0.851805
        ,  0.868675
        ,  0.885955
    };

    cpt::algorithm::BRLMMp brlmmp( 3 );

    auto stime = get_time_point();
    brlmmp.find_min_bic( vec );
    auto etime = get_time_point();

    output_bic( brlmmp.buffers[0], vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_real_case_3 )
{
    std::vector< double > vec{
          -0.628385
        , -0.616352
        , -0.602444
        , -0.593783
        , -0.532464
        , -0.509005
        , -0.391085
        ,  0.301951
        ,  0.304787
        ,  0.312591
        ,  0.316924
        ,  0.319686
        ,  0.330808
        ,  0.347661
        ,  0.348108
        ,  0.350953
        ,  0.35594
        ,  0.356953
        ,  0.362956
        ,  0.379373
        ,  0.379774
        ,  0.380058
        ,  0.385081
        ,  0.393584
        ,  0.408447
        ,  0.409054
        ,  0.414259
        ,  0.416277
        ,  0.423403
        ,  0.423884
        ,  0.43024
        ,  0.430804
        ,  0.453445
        ,  0.456521
        ,  0.460196
        ,  0.473854
        ,  0.477136
        ,  0.48567
        ,  0.490417
        ,  0.491551
        ,  0.496556
        ,  0.502874
        ,  0.535856
        ,  0.536627
        ,  0.558241
        ,  0.571606
        ,  0.581046
        ,  0.586338
        ,  0.609275
        ,  0.610404
        ,  0.761889
        ,  0.784956
        ,  0.798665
        ,  0.805654
        ,  0.808581
        ,  0.811739
        ,  0.812896
        ,  0.816824
        ,  0.817921
        ,  0.819855
        ,  0.820853
        ,  0.821825
        ,  0.823281
        ,  0.823969
        ,  0.826464
        ,  0.82952
        ,  0.829653
        ,  0.830512
        ,  0.831099
        ,  0.831701
        ,  0.833809
        ,  0.834257
        ,  0.83429
        ,  0.834485
        ,  0.834963
        ,  0.837774
        ,  0.838024
        ,  0.839598
        ,  0.842128
        ,  0.842233
        ,  0.843543
        ,  0.84748
        ,  0.850521
        ,  0.850557
        ,  0.851689
        ,  0.852918
        ,  0.857871
        ,  0.859795
        ,  0.860193
        ,  0.863629
        ,  0.864189
        ,  0.866582
        ,  0.87114
        ,  0.875466
        ,  0.877692
        ,  0.879367
        ,  0.886485
        ,  0.889427
        ,  0.901585
        ,  0.902399
    };

    std::vector< double > likelihoods;

    for( size_t i = 1; i <= 3; ++i )
    {
        cpt::algorithm::BRLMMpClustering bc( vec, i );

        auto stime = get_time_point();
        auto ml = bc.run();
        auto etime = get_time_point();

        likelihoods.emplace_back( ml.first );
        output_bdy( ml, vec );
        count( stime, etime );
    }

    std::cout << "MinBic";

    for( size_t k = likelihoods.size(); k != 0; --k )
    {
        double bic = -2 - likelihoods[ k-1 ] + ( 3 * k - 1 ) * log( vec.size() );
        std::cout << "\t" << k << ":" << bic;
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, find_MinBIC_real_case_3 )
{
    std::vector< double > vec{
          -0.628385
        , -0.616352
        , -0.602444
        , -0.593783
        , -0.532464
        , -0.509005
        , -0.391085
        ,  0.301951
        ,  0.304787
        ,  0.312591
        ,  0.316924
        ,  0.319686
        ,  0.330808
        ,  0.347661
        ,  0.348108
        ,  0.350953
        ,  0.35594
        ,  0.356953
        ,  0.362956
        ,  0.379373
        ,  0.379774
        ,  0.380058
        ,  0.385081
        ,  0.393584
        ,  0.408447
        ,  0.409054
        ,  0.414259
        ,  0.416277
        ,  0.423403
        ,  0.423884
        ,  0.43024
        ,  0.430804
        ,  0.453445
        ,  0.456521
        ,  0.460196
        ,  0.473854
        ,  0.477136
        ,  0.48567
        ,  0.490417
        ,  0.491551
        ,  0.496556
        ,  0.502874
        ,  0.535856
        ,  0.536627
        ,  0.558241
        ,  0.571606
        ,  0.581046
        ,  0.586338
        ,  0.609275
        ,  0.610404
        ,  0.761889
        ,  0.784956
        ,  0.798665
        ,  0.805654
        ,  0.808581
        ,  0.811739
        ,  0.812896
        ,  0.816824
        ,  0.817921
        ,  0.819855
        ,  0.820853
        ,  0.821825
        ,  0.823281
        ,  0.823969
        ,  0.826464
        ,  0.82952
        ,  0.829653
        ,  0.830512
        ,  0.831099
        ,  0.831701
        ,  0.833809
        ,  0.834257
        ,  0.83429
        ,  0.834485
        ,  0.834963
        ,  0.837774
        ,  0.838024
        ,  0.839598
        ,  0.842128
        ,  0.842233
        ,  0.843543
        ,  0.84748
        ,  0.850521
        ,  0.850557
        ,  0.851689
        ,  0.852918
        ,  0.857871
        ,  0.859795
        ,  0.860193
        ,  0.863629
        ,  0.864189
        ,  0.866582
        ,  0.87114
        ,  0.875466
        ,  0.877692
        ,  0.879367
        ,  0.886485
        ,  0.889427
        ,  0.901585
        ,  0.902399
    };

    cpt::algorithm::BRLMMp brlmmp( 3 );

    auto stime = get_time_point();
    brlmmp.find_min_bic( vec );
    auto etime = get_time_point();

    output_bic( brlmmp.buffers[0], vec );
    count( stime, etime );
}

TEST( brlmmp_test, clustering_real_case_4 )
{
    std::vector< double > vec{
          -0.905988
        , -0.895262
        , -0.892088
        , -0.887147
        , -0.882629
        , -0.881914
        , -0.8814
        , -0.877572
        , -0.870111
        , -0.866821
        , -0.833396
        , -0.824011
        , -0.280591
        , -0.193698
        , -0.178455
        , -0.166155
        , -0.160562
        , -0.135197
        , -0.0943189
        , -0.0796395
        , -0.0534936
        , -0.036485
        , -0.0353174
        ,  0.0204562
        ,  0.0280624
        ,  0.0379625
        ,  0.835034
        ,  0.855899
        ,  0.873422
        ,  0.894913
    };

    std::vector< double > likelihoods;

    for( size_t i = 1; i <= 3; ++i )
    {
        cpt::algorithm::BRLMMpClustering bc( vec, i );

        auto stime = get_time_point();
        auto ml = bc.run();
        auto etime = get_time_point();

        likelihoods.emplace_back( ml.first );
        output_bdy( ml, vec );
        count( stime, etime );
    }

    std::cout << "MinBic";

    for( size_t k = likelihoods.size(); k != 0; --k )
    {
        double bic = -2 - likelihoods[ k-1 ] + ( 3 * k - 1 ) * log( vec.size() );
        std::cout << "\t" << k << ":" << bic;
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, find_MinBIC_real_case_4 )
{
    std::vector< double > vec{
          -0.905988
        , -0.895262
        , -0.892088
        , -0.887147
        , -0.882629
        , -0.881914
        , -0.8814
        , -0.877572
        , -0.870111
        , -0.866821
        , -0.833396
        , -0.824011
        , -0.280591
        , -0.193698
        , -0.178455
        , -0.166155
        , -0.160562
        , -0.135197
        , -0.0943189
        , -0.0796395
        , -0.0534936
        , -0.036485
        , -0.0353174
        ,  0.0204562
        ,  0.0280624
        ,  0.0379625
        ,  0.835034
        ,  0.855899
        ,  0.873422
        ,  0.894913
    };

    cpt::algorithm::BRLMMp brlmmp( 3 );

    auto stime = get_time_point();
    brlmmp.find_min_bic( vec );
    auto etime = get_time_point();

    output_bic( brlmmp.buffers[0], vec );
    count( stime, etime );
}

TEST( brlmmp_test, find_min_bic )
{
    std::vector< double > vec3{ -2.05, -2, -1.95, -0.05, 0, 0.05, 1.95, 2, 2.05 };
    std::vector< double > vec2{ -2.05, -2, -1.95, 0, 0.01, 0.02, 0.03, 0.04 };
    std::vector< double > vec1{ 2.8, 2.9, 3, 3, 3 };

    std::cout << "find min bic";

    cpt::algorithm::BRLMMp brlmmp( 3 );

    brlmmp.find_min_bic( vec3 );
    brlmmp.find_min_bic( vec2 );
    brlmmp.find_min_bic( vec1 );

    if( brlmmp.buffers[0].boundarys.size() == 2 &&
        brlmmp.buffers[1].boundarys.size() == 1 &&
        brlmmp.buffers[2].boundarys.size() == 0 )
    {
        std::cout << "\rfind min bic complete";
    }
    else
    {
        std::cout << "\rfind min bic fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, gmm_bic_to_python )
{
    std::vector< double > vec{ -2.05, -2, -1.95, -0.05, 0, 0.05, 1.95, 2, 2.05 };

    cpt::algorithm::BRLMMp brlmmp( 3 );
    cpt::algorithm::BRLMMpClustering bc( vec, 3 );

    auto ml = bc.run();
    auto bic = brlmmp.bic( ml.first, 3, vec.size() );

    std::cout << "ML: " << ml.first << "\tBIC: " << bic << "\nMean:";

    for( auto& mean : ml.second.means )
    {
        std::cout << "\t" << mean;
    }

    std::cout << "\nStd:";

    for( auto& std : ml.second.stds )
    {
        std::cout << "\t" << std;
    }

    std::cout << "\nWeight:";

    for( auto& th : ml.second.thetas )
    {
        std::cout << "\t" << th;
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, incremental_mean_estimate )
{
    std::cout << "incremental mean";
    cpt::algorithm::BRLMMp brlmmp( 3 );

    if( brlmmp.incremental_mean_estimate( 5, 10, 10 ) == 5.5 )
    {
        std::cout << "\rincremental mean complete";
    }
    else
    {
        std::cout << "\rincremental mean fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, variances_estimate_for_mean_or_std )
{
    std::vector< std::vector< double >> res( 3, std::vector< double >( 3, 0 ));
    std::vector< std::vector< double >> vecs{{ 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }};
    std::vector< double > mean{ 2, 5, 8 };

    std::cout << "variances estimate for mean or std";

    cpt::algorithm::BRLMMp brlmmp( 3 );
    std::cerr << "\n";

    for( auto& vec : vecs )
        brlmmp.variances_estimate_for_mean_or_std( 3, res, mean, vec ); 

    for( size_t a = 0; a < res.size(); ++a )
    {
        for( size_t b = 0; b < res.size(); ++b )
            std::cerr << "\t" << res[a][b] / 2;
        std::cerr << "\n";
    }

    std::cerr << "\n";
}

TEST( brlmmp_test, build_model )
{
    std::vector< std::vector< double >> vecs
    {
          { -1.00, -0.92, -0.90, -0.10,  0.02,  0.10,  0.90,  0.92,  1.00 }
        , { -1.00, -0.94, -0.90, -0.10,  0.04,  0.10,  0.90,  0.94,  1.00 }
        , { -1.00, -0.96, -0.90, -0.10,  0.06,  0.10,  0.90,  0.98,  1.00 }
        , { -1.00, -0.98, -0.90, -0.10, -0.08,  0.10,  0.90,  0.99,  1.00 }
    };

    std::cout << "build_model";
    cpt::algorithm::BRLMMp brlmmp( 3 );

    for( auto& vec : vecs )
        brlmmp.find_min_bic( vec );

    brlmmp.build_model( brlmmp.buffers );
    brlmmp.print_test();
    brlmmp.print_of_test();
}

TEST( brlmmp_test, imputate_for_1_cluster_case1 )
{
    std::vector< std::vector< double >> vecs
    {
          { -0.10, -0.03,  0.03,  0.10,  0.90,  0.93,  0.95,  0.97,  1.00 }
        , { -1.00, -0.91, -0.90, -0.10,  0.07,  0.10,  0.90,  0.99,  1.00 }
        , { -1.00, -0.93, -0.90, -0.10,  0.05,  0.10,  0.90,  0.97,  1.00 }
        , { -1.00, -0.95, -0.90, -0.10,  0.00,  0.10,  0.90,  0.95,  1.00 }
        , { -1.00, -0.97, -0.90, -0.10, -0.05,  0.10,  0.90,  0.93,  1.00 }
        , { -1.00, -0.99, -0.90, -0.10, -0.07,  0.10,  0.90,  0.91,  1.00 }
    };

    std::cout << "imputate for 1 cluster case 1";
    cpt::algorithm::BRLMMp brlmmp( 3 );

    for( auto& vec : vecs )
        brlmmp.find_min_bic( vec );

    brlmmp.build_model( brlmmp.buffers );
    brlmmp.print_test();

    brlmmp.imputate_for_1_cluster( 1, brlmmp.buffers[0].means, brlmmp.buffers[0].means_of_mean );
    brlmmp.imputate_for_1_cluster( 1, brlmmp.buffers[0].stds, brlmmp.buffers[0].means_of_std );
    
    std::cerr << "\n";
    brlmmp.print_test();
}

TEST( brlmmp_test, imputate_for_1_cluster_case3 )
{
    std::vector< std::vector< double >> vecs
    {
          { -1.00, -0.97, -0.95, -0.93, -0.90, -0.10, -0.03,  0.03,  0.10 }
        , { -1.00, -0.91, -0.90, -0.10,  0.07,  0.10,  0.90,  0.99,  1.00 }
        , { -1.00, -0.93, -0.90, -0.10,  0.05,  0.10,  0.90,  0.97,  1.00 }
        , { -1.00, -0.95, -0.90, -0.10,  0.00,  0.10,  0.90,  0.95,  1.00 }
        , { -1.00, -0.97, -0.90, -0.10, -0.05,  0.10,  0.90,  0.93,  1.00 }
        , { -1.00, -0.99, -0.90, -0.10, -0.07,  0.10,  0.90,  0.91,  1.00 }
    };

    std::cout << "imputate for 1 cluster case 3";
    cpt::algorithm::BRLMMp brlmmp( 3 );

    for( auto& vec : vecs )
        brlmmp.find_min_bic( vec );

    brlmmp.build_model( brlmmp.buffers );
    brlmmp.print_test();

    brlmmp.imputate_for_1_cluster( 3, brlmmp.buffers[0].means, brlmmp.buffers[0].means_of_mean );
    brlmmp.imputate_for_1_cluster( 3, brlmmp.buffers[0].stds, brlmmp.buffers[0].means_of_std );
    
    std::cerr << "\n";
    brlmmp.print_test();
}

TEST( brlmmp_test, imputate_for_2_cluster_case1 )
{
    std::vector< std::vector< double >> vecs
    {
          { -1.00, -0.99, -0.98, -0.96, -0.95, -0.94, -0.92, -0.91, -0.90 }
        , { -1.00, -0.91, -0.90, -0.10,  0.07,  0.10,  0.90,  0.99,  1.00 }
        , { -1.00, -0.93, -0.90, -0.10,  0.05,  0.10,  0.90,  0.97,  1.00 }
        , { -1.00, -0.95, -0.90, -0.10,  0.00,  0.10,  0.90,  0.95,  1.00 }
        , { -1.00, -0.97, -0.90, -0.10, -0.05,  0.10,  0.90,  0.93,  1.00 }
        , { -1.00, -0.99, -0.90, -0.10, -0.07,  0.10,  0.90,  0.91,  1.00 }
    };

    std::cout << "imputate for 2 cluster case 1";
    cpt::algorithm::BRLMMp brlmmp( 3 );

    for( auto& vec : vecs )
        brlmmp.find_min_bic( vec );

    brlmmp.build_model( brlmmp.buffers );
    brlmmp.print_test();

    brlmmp.imputate_for_2_cluster( 1, brlmmp.buffers[0].means, brlmmp.buffers[0].means_of_mean );
    brlmmp.imputate_for_2_cluster( 1, brlmmp.buffers[0].stds, brlmmp.buffers[0].means_of_std );
    
    std::cerr << "\n";
    brlmmp.print_test();
}

TEST( brlmmp_test, imputate_for_2_cluster_case3 )
{
    std::vector< std::vector< double >> vecs
    {
          {  0.90,  0.91,  0.92,  0.94,  0.95,  0.96,  0.97,  0.99,  1.00 }
        , { -1.00, -0.91, -0.90, -0.10,  0.07,  0.10,  0.90,  0.99,  1.00 }
        , { -1.00, -0.93, -0.90, -0.10,  0.05,  0.10,  0.90,  0.97,  1.00 }
        , { -1.00, -0.95, -0.90, -0.10,  0.00,  0.10,  0.90,  0.95,  1.00 }
        , { -1.00, -0.97, -0.90, -0.10, -0.05,  0.10,  0.90,  0.93,  1.00 }
        , { -1.00, -0.99, -0.90, -0.10, -0.07,  0.10,  0.90,  0.91,  1.00 }
    };

    std::cout << "imputate for 2 cluster case 3";
    cpt::algorithm::BRLMMp brlmmp( 3 );

    for( auto& vec : vecs )
        brlmmp.find_min_bic( vec );

    brlmmp.build_model( brlmmp.buffers );
    brlmmp.print_test();

    brlmmp.imputate_for_2_cluster( 3, brlmmp.buffers[0].means, brlmmp.buffers[0].means_of_mean );
    brlmmp.imputate_for_2_cluster( 3, brlmmp.buffers[0].stds, brlmmp.buffers[0].means_of_std );

    std::cerr << "\n";
    brlmmp.print_test();
}

TEST( brlmmp_test, imputate_1_missing_cluster__missing_cluster_1 )
{
    std::vector< double > vec{ -0.04, -0.03, -0.02, -0.01, 0, 0.05, 1.95, 2, 2.05 };
    std::cout << "missing cluster 1";

    cpt::algorithm::BRLMMp brlmmp( 3 );
    brlmmp.find_min_bic( vec );

    double boundary_1 = brlmmp.buffers[0].means[0] + 1;
    double boundary_2 = 1 - brlmmp.buffers[0].means[1];

    if( boundary_2 < boundary_1 )
    {    
        std::cout << "\rmissing cluster 1 complete";
    }
    else
    {
        std::cout << "\rmissing cluster 1 fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, imputate_1_missing_cluster__missing_cluster_3 )
{
    std::vector< double > vec{ -2.05, -2, -1.95, 0, 0.01, 0.02, 0.03, 0.04 };
    std::cout << "missing cluster 3";

    cpt::algorithm::BRLMMp brlmmp( 3 );
    brlmmp.find_min_bic( vec );

    double boundary_1 = brlmmp.buffers[0].means[0] + 1;
    double boundary_2 = 1 - brlmmp.buffers[0].means[1];

    if( boundary_2 > boundary_1 )
    {
        std::cout << "\rmissing cluster 3 complete";
    }
    else
    {
        std::cout << "\rmissing cluster 3 fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, imputate_2_missing_cluster__missing_cluster_1_2 )
{
    std::vector< double > vec{ 0.8, 0.85, 0.9, 0.95, 1 };
    std::cout << "missing cluster 1/2";

    cpt::algorithm::BRLMMp brlmmp( 3 );
    brlmmp.find_min_bic( vec );

    double boundary_1 = brlmmp.buffers[0].data_range.first + 1;
    double boundary_2 = 1 - brlmmp.buffers[0].data_range.second;

    if( boundary_2 < boundary_1 )
    {
        std::cout << "\rmissing cluster 1/2 complete";
    }
    else
    {
        std::cout << "\rmissing cluster 1/2 fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, imputate_2_missing_cluster__missing_cluster_2_3 )
{
    std::vector< double > vec{ -1, -0.95, -0.9, -0.85, -0.8 };
    std::cout << "missing cluster 2/3";

    cpt::algorithm::BRLMMp brlmmp( 3 );
    brlmmp.find_min_bic( vec );

    double boundary_1 = brlmmp.buffers[0].data_range.first + 1;
    double boundary_2 = 1 - brlmmp.buffers[0].data_range.second;

    if( boundary_2 > boundary_1 )
    {
        std::cout << "\rmissing cluster 2/3 complete";
    }
    else
    {
        std::cout << "\rmissing cluster 2/3 fail";
    }

    std::cout << std::endl;
}

TEST( brlmmp_test, brlmmp_basic_testing )
{
    std::vector< double > vec{ -2.05, -2, -1.95, -0.05, 0, 0.05, 1.95, 2, 2.05 };

    cpt::algorithm::BRLMMp brlmmp( 3 );
    auto stime = get_time_point();

    brlmmp.find_min_bic( vec );
    brlmmp.build_model( brlmmp.buffers );

    for( auto& data : vec )
    {
        // XXX
        std::cerr << data << "\t" << brlmmp.genotyping( 0, data, brlmmp.buffers ).first << "\t" << brlmmp.genotyping( 0, data, brlmmp.buffers ).second << "\n";
    }

    auto etime = get_time_point();
    count( stime, etime );
}

TEST( brlmmp_test, brlmmp_case_1 )
{
    std::vector< std::pair< double, size_t >> datas( 
            get_file( "/home/alex/Centrillion/CPT/unit_test/data/case_1.txt" ));

    std::vector< double > vec( to_data_vector( datas ));

    cpt::algorithm::BRLMMp brlmmp( 3 );
    auto stime = get_time_point();

    brlmmp.find_min_bic( vec );
    brlmmp.build_model( brlmmp.buffers );

    for( auto& data : vec )
    {
        // XXX
        std::cerr << data << "\t" << brlmmp.genotyping( 0, data, brlmmp.buffers ).first << "\t" << brlmmp.genotyping( 0, data, brlmmp.buffers ).second << "\n";
    }
    auto etime = get_time_point();

    count( stime, etime );
}
*/
