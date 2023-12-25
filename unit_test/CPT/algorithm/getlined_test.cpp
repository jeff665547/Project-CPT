
#include <Nucleona/app/cli/gtest.hpp> 
#include <CPT/algorithm/getlined.hpp>
#include <fstream>
#include <string>
#ifndef PROJECT_ROOT
#define PROJECT_ROOT "./"
#endif 

RANGE_NAMESPACE_SHORTCUT
using namespace std::string_literals;
TEST(getlined, filetest)
{
    for( auto&& line : std::ifstream(
        PROJECT_ROOT + "/unit_test/data/algorithm/getlined_test.txt"s
    ) | ca::getlined() )
    {
        std::cout << line << std::endl;
    }
}
