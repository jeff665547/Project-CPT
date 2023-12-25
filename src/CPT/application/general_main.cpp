#define STR(s) #s
#define MAIN_HPP(APP) STR(CPT/application/APP/main.hpp)
#include MAIN_HPP(APP_NAME)
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
//
int main ( int argc, const char* argv[] )
{
    cpt::application::APP_NAME::OptionParser option_parser(argc, argv);
    auto&& main = cpt::application::APP_NAME::make(option_parser);
    main();
    return 0;
}
