#include <unistd.h>
#include <CPT/application/il2cel/main.hpp>
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
int main ( int argc, const char* argv[] )
{
    cpt::application::il2cel::OptionParser option_parser(argc, argv);
    auto&& main = cpt::application::il2cel::make(option_parser);
    main();
    return 0;
}
