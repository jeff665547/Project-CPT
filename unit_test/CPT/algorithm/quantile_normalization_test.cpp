#include <Nucleona/app/cli/gtest.hpp>
#include <iostream>
#include <CPT/algorithm/quantile_normalization_for_cube.hpp>

TEST( quantile_normalization_test, Functional_testing_1 )
{
    nucleona::parallel::set_max_thread_num((int)std::thread::hardware_concurrency());
    arma::Mat< double > mat = {
        {  1, 11, 13, 29, 26 },
        { 15, 17,  5,  8, 14 },
        { 21,  2, 12, 20, 25 },
        { 10, 19, 16, 24,  4 },
        { 18, 28,  3, 22, 27 },
        {  7, 23, 30,  6,  9 }
    };

    cpt::format::Cube< double > cube( 6, 5, 1, arma::fill::zeros );
    cube.slice( 0 ) = mat;

    cube.print( "\nQuantile without quantile:" );

    cpt::format::Cube< double > qube( cube );
    std::vector< double > sketch_table;
    cpt::algorithm::QuantileNormalizationForCube< double > quantile( 
        qube, sketch_table, 0, 1 
    );

    quantile.target_sketch_estimate();
    quantile.do_quantile_normalization();

    qube.print( "\nCube with quantile:" );
}

TEST( quantile_normalization_test, Functional_testing_2 )
{
    nucleona::parallel::set_max_thread_num((int)std::thread::hardware_concurrency());
    arma::Mat< double > mat = {
        {  1, 11, 13, 29 },
        { 15, 17,  5,  8 },
        { 21,  2, 12, 20 },
        { 10, 19, 16, 24 },
        { 18, 28,  3, 22 },
        {  7, 23, 30,  6 }
    };

    cpt::format::Cube< double > cube( 6, 4, 1, arma::fill::zeros );
    cube.slice( 0 ) = mat;

    cube.print( "\nQuantile without quantile:" );

    cpt::format::Cube< double > qube( cube );
    std::vector< double > sketch_table;
    cpt::algorithm::QuantileNormalizationForCube< double > quantile( qube, sketch_table, 0 );

    quantile.target_sketch_estimate();
    quantile.do_quantile_normalization();

    qube.print( "\nCube with quantile:" );
}

TEST( quantile_normalization_test, Functional_testing_3 )
{
    nucleona::parallel::set_max_thread_num((int)std::thread::hardware_concurrency());
    arma::Mat< double > mat1 = {
        {  1, 11 },
        { 15, 17 },
        { 21,  2 },
        { 10, 19 },
        { 18, 28 },
        {  7, 23 }
    };

    arma::Mat< double > mat2 = {
        { 13, 29 },
        {  5,  8 },
        { 12, 20 },
        { 16, 24 },
        {  3, 22 },
        { 30,  6 }
    };

    cpt::format::Cube< double > cube( 6, 2, 2, arma::fill::zeros );
    cube.slice( 0 ) = mat1;
    cube.slice( 1 ) = mat2;

    cube.print( "\nQuantile without quantile:" );

    cpt::format::Cube< double > qube( cube );
    std::vector< double > sketch_table;
    cpt::algorithm::QuantileNormalizationForCube< double > quantile( qube, sketch_table, 0 );

    quantile.target_sketch_estimate();
    quantile.do_quantile_normalization();

    qube.print( "\nCube with quantile:" );
}

// TEST( quantile_normalization_test, quantile_normalization_testing )
// {
//     cpt::format::Cube< double > cube( 10, 5, 2, arma::fill::randu );
//     cube.fill_axis_name( "Probesets", "Samples", "Channels" );
// 
//     cube.x_axis.set_labels( std::vector< std::string >({ "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P9" }));
//     cube.y_axis.set_labels( std::vector< std::string >({ "S0", "S1", "S2", "S3", "S4" }));
//     cube.z_axis.set_labels( std::vector< std::string >({ "C0", "C1" }));
// 
//     cube.print_axis_name();
//     cube.print_axis_labels();
//     cube.print( "\nCube with no quantile:" );
// 
//     // Cube< double > qube( cube );
//     // std::vector< double > sketch_table;
//     // cpt::algorithm::QuantileNormalizationForCube< double > quantile( qube, sketch_table, 0 );
// 
//     // qube.print_axis_name();
//     // qube.print_axis_labels();
//     // qube.print( "\nCube with quantile:" );
// 
//     // Cube< double > qubei( cube );
//     // cpt::algorithm::QuantileNormalizationForCube< double > qtl( qubei, sketch_table );
// 
//     // qubei.print_axis_name();
//     // qubei.print_axis_labels();
//     // qubei.print( "\nQuantile with sketch_table:" );
// 
//     // Cube< double > qubeii( cube );
//     // cpt::algorithm::QuantileNormalizationForCube< double > qt2( qubei, sketch_table, 0 );
// 
//     // qubeii.print_axis_name();
//     // qubeii.print_axis_labels();
//     // qubeii.print( "\nQuantile with sketch_table:" );
// 
//     cpt::format::Cube< double > qube( cube );
//     std::vector< double > sketch_table;
//     cpt::algorithm::QuantileNormalizationForCube< double > quantile( qube, sketch_table, 0 );
// 
//     quantile.target_sketch_estimate();
//     quantile.do_quantile_normalization();
// 
//     qube.print_axis_name();
//     qube.print_axis_labels();
//     qube.print( "\nCube with quantile:" );
// 
//     cpt::format::Cube< double > qubei( cube );
//     cpt::algorithm::QuantileNormalizationForCube< double > qtl( qubei, sketch_table );
// 
//     quantile.target_sketch_estimate();
//     quantile.do_quantile_normalization();
// 
//     qubei.print_axis_name();
//     qubei.print_axis_labels();
//     qubei.print( "\nQuantile with sketch_table:" );
// }
