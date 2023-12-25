#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/utility/irange.hpp>
#include <CPT/algorithm/grouped.hpp>
namespace cu = cpt::utility;
namespace ca = cpt::algorithm;
TEST(group, range_test)
{
    for ( auto&& group_rng : cu::irange_0(10) | ca::grouped(2) )
    {
    }
}
