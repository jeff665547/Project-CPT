#include <CPT/application/improc2/main.hpp>
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>

int main(int argc, char* argv[])
{
    cpt::application::improc2::OptionParser option_parser(argc, argv);
    auto&& main = cpt::application::improc2::make(option_parser);
    main();
    return 0;
}

