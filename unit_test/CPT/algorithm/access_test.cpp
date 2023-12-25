#include <CPT/algorithm/access.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <vector>
#include <string>
#include <CPT/utility/print.hpp>
#include <CPT/algorithm/print.hpp>
#include <boost_addon/range_eval.hpp>
#include <boost/range/combine.hpp>
namespace ca = cpt::algorithm;
namespace cu = cpt::utility;
namespace ba = boost::adaptors;
std::vector<std::string> strv {
    "aaaa", "bbbb", "cccc", "dddd"
    , "eeee", "ffff"
};
TEST(idx_access, view)
{
    strv | ca::printed(std::cout, '\t') | endp; // print strv all
    std::cout << std::endl;
};
TEST( idx_access, one_level_test )
{
    std::vector<std::string> expect { "bbbb", "cccc", "eeee", "aaaa" };
    auto ans = strv 
        | ca::idx_access({1, 2, 4, 0}) 
        | to_vector;
    /* result : bbbb\tcccc\teeee\taaaa */
    for ( auto&& p : boost::combine( ans, expect ) )
    {
        EXPECT_EQ( p.get<0>(), p.get<1>() );
    }
}
TEST( idx_access, multi_level_test )
{
    std::vector<std::string> expect { "cccc", "eeee" };
    auto ans = strv 
        | ca::idx_access({1, 2, 4, 0})  /* bbbb cccc eeee aaaa */
        | ca::idx_access({1, 2})        /* cccc eeee */
        | to_vector
    ;
    for ( auto&& i : ans )
    {
        std::cout << i << std::endl;
    }
    /* result : cccc\teeee */
    for ( auto&& p : boost::combine( ans, expect ) )
    {
        EXPECT_EQ( p.get<0>(), p.get<1>() );
    }
}
TEST( idx_access, write_test )
{
    std::vector<std::string> expect { "zzzz", "zzzz", "zzzz", "dddd", "zzzz", "ffff" };
    auto qq = strv 
        | ca::idx_access({1, 2, 4, 0}) 
        | ca::lazy_foreach(
            [] ( auto& v ) -> auto&
            {
                v = "zzzz";
                return v;
            }
        ) 
        | endp
    ;
    /* result : zzzz\tzzzz\tzzzz\tdddd\tzzzz\tffff */
    for ( auto&& p : boost::combine( strv, expect ) )
    {
        EXPECT_EQ( p.get<0>(), p.get<1>() );
    }
}
