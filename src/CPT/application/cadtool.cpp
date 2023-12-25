#include <CPT/application/cadtool/main.hpp>
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
int main ( int argc, const char* argv[] )
{
    cpt::application::cadtool::OptionParser option_parser(argc, argv);
    auto&& main = cpt::application::cadtool::make(option_parser);
    main();
    return 0;
}
