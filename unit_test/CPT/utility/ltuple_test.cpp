#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/utility/ltuple.hpp>
class TestIdx;
MAKE_LTUPLE_HEADER(TestIdx, "qq", "ww", "mm", "an")
TEST(LTuple, basic_test)
{
    namespace cu = cpt::utility;
    typedef cu::LTuple<TestIdx, int, double, uint64_t, std::string> LTP;
    LTP t(15, 0.4, 50, "dog");


    // EXPECT_EQ( 15, cpt::utility::get("qq", t) );
    EXPECT_EQ( 15       , LTGET("qq", t) );
    EXPECT_EQ( 0.4      , LTGET("ww", t) );
    EXPECT_EQ( 50       , LTGET("mm", t) );
    EXPECT_EQ( "dog"    , LTGET("an", t) );
    
}
