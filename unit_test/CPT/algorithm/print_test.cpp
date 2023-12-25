#include <CPT/algorithm/print.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <sstream>
#include <boost_addon/range_eval.hpp>
TEST( print, basic_test )
{
    std::stringstream ss;

    std::vector<int>({ 10, 20, 30, 40, 50 })
    | cpt::algorithm::printed( ss, '\t' )
    | endp
    ;
    std::cout << ss.str() << std::endl;
    EXPECT_EQ(ss.str(), "10\t20\t30\t40\t50");
}
TEST( print, multi_layer )
{
    std::stringstream ss;
    std::vector<int> vec ({ 10, 20, 30, 40, 50 });
    vec
    | cpt::algorithm::lazy_foreach(
        []( auto& i ) -> auto&
        {
            i += i;
            return i;
        }
    )
    | cpt::algorithm::lazy_foreach(
        []( auto& j ) -> auto
        {
            return j / 10;
        }
    )
    | cpt::algorithm::printed( ss, '\n' )
    | endp
    ;
    EXPECT_EQ( ss.str(), "2\n4\n6\n8\n10" );
}
