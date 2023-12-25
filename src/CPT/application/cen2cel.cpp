#include <CPT/application/cen2cel/main.hpp>
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
int main ( int argc, const char* argv[] )
{
    cpt::application::cen2cel::OptionParser option_parser(argc, argv);
    auto&& main = cpt::application::cen2cel::make(option_parser);
    main();
    return 0;
}
