#include <CPT/application/cdf2cad/main.hpp>
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
int main ( int argc, const char* argv[] )
{
    cpt::application::cdf2cad::OptionParser option_parser(argc, argv);
    auto&& main = cpt::application::cdf2cad::make(option_parser);
    main();
    return 0;
}
