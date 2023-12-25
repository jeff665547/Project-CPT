#include <CPT/application/intensities_dump/main.hpp> 

int main( int argc, const char* argv[] )
{
    cpt::application::intensities_dump::OptionParser option_parser(argc, argv);
    auto&& intensities_dumper ( 
        cpt::application::intensities_dump::make( option_parser ) 
    );
    intensities_dumper();
    return 0;
}
