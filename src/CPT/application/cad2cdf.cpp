#include <unistd.h>
#include <CPT/application/cad2cdf/main.hpp>
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
int main ( int argc, const char* argv[] )
{
    cpt::application::cad2cdf::OptionParser option_parser(argc, argv);
    auto&& main = cpt::application::cad2cdf::make(option_parser);
    main();
    return 0;
}
