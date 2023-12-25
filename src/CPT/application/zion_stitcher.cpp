#include <iostream> 
#include <string>
#include <CPT/application/zion_stitcher/main.hpp>
int main( int argc, const char* argv[] )
{
    cpt::application::summit_stitch::OptionParser option_parser(argc, argv);    
    auto&& stitcher( 
        cpt::application::summit_stitch::make( option_parser )
    );
    stitcher();
}