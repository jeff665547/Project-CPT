#include <boost_addon/function_range.hpp>
#include <Nucleona/app/cli/gtest.hpp>
int mul2( int i )
{return i*2;}
auto mylog(int i)
{
    return std::log(i);
}
TEST(function_range, basic_test)
{
    auto range( ::make_rnd_func_rng(mylog, 1, 10));
    for( auto& i : range )
    {
        std::cout << i << std::endl;
    }
}
