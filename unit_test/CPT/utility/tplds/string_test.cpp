#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/utility/tplds/string.hpp>
#include <string>

template<class T>
auto get_type()
{
    return std::string(__PRETTY_FUNCTION__);
};
TEST( tpl_string, basic_test )
{
    using HelloWorld = TPL_STR("hello_world");
    std::cout << get_type<HelloWorld>() << std::endl;
    std::cout << HelloWorld::to_stl_string() << std::endl;
    EXPECT_STREQ(
        get_type<HelloWorld>().c_str()
        , "auto get_type() [with T = cpt::utility::tplds::String<'h', 'e', 'l', 'l', 'o', '_', 'w', 'o', 'r', 'l', 'd'>]"
    );
    EXPECT_STREQ(HelloWorld::to_stl_string().c_str(), "hello_world");

    using PITYPE = TPL_STR("3.1416");
    std::cout << get_type<PITYPE>() << std::endl;
    std::cout << PITYPE::to_stl_string() << std::endl;
    EXPECT_STREQ(
        get_type<PITYPE>().c_str()
        , "auto get_type() [with T = cpt::utility::tplds::String<'3', '.', '1', '4', '1', '6'>]"
    );
    EXPECT_STREQ(PITYPE::to_stl_string().c_str(), "3.1416");
}
