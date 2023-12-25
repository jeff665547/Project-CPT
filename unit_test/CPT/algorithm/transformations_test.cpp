#include <Nucleona/app/cli/gtest.hpp>
#include <iostream>
#include <CPT/algorithm/transformations.hpp>

TEST( quantile_normalization_test, quantile_normalization_testing )
{
    cpt::format::Cube< double > cube( 4, 3, 2, arma::fill::randu );
    cube.print( "Cube" );

    cpt::format::Cube< double > qube1( cube );
    cpt::algorithm::Transformations trans1( qube1 );
    trans1.ma();
    qube1.print( "MvA" );

    cpt::format::Cube< double > qube2( cube );
    cpt::algorithm::Transformations trans2( qube2 );
    trans2.rt();
    qube1.print( "RvT" );

    cpt::format::Cube< double > qube3( cube );
    cpt::algorithm::Transformations trans3( qube3 );
    trans3.ccs();
    qube3.print( "Contrast Centers Stretch" );

    cpt::format::Cube< double > qube4( cube );
    cpt::algorithm::Transformations trans4( qube4 );
    trans4.ces();
    qube4.print( "Contrast Extremes Stretch" );
}
