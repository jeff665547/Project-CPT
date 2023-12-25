#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/algorithm/argmin.hpp>
#include <CPT/algorithm/access.hpp>
#include <CPT/algorithm/print.hpp>
#include <boost_addon/range_eval.hpp>
#include <boost/range/irange.hpp>
namespace ca = cpt::algorithm;
double curve( int o )
{
    return o*o;
}
TEST(argmin_test, funcion)
{
    std::vector<int> values {
        -2, -4, 6, 7, 1, -3, 5
    };
    auto min_arg( values | ca::argmind(curve) );
    EXPECT_EQ(min_arg, 1 );
}
TEST(argmin_test, index)
{
    std::vector<int> values {
        -2, -4, 6, 7, 1, -3, 5
    };
    auto min_arg( boost::irange(std::size_t(0), values.size()) | ca::argmind(
        [values](auto i)
        {
            return values[i];
        }
    ) );
    
    EXPECT_EQ(min_arg, 1 );
}
TEST(argmin_test, index2)
{
    std::vector<int> values {
        -2, -4, 6, 7, 1, -3, 5
    };
    auto min_arg( values | ca::indexmind() );
    EXPECT_EQ(min_arg, 1 );
}
