// #include <Nucleona/app/cli/gtest.hpp>
#include <boost/range/adaptors.hpp>
#include <CPT/algorithm/state_flattened.hpp>
#include <boost_addon/range_vector.hpp>
#include <boost_addon/range_glue.hpp>
namespace ca = cpt::algorithm;
namespace ba = boost::adaptors;
template<class T>
constexpr bool is_range_f(T&& o)
{
    return is_range_v<std::decay_t<T>>;
}
struct QQ
{
    auto begin(){};
    auto end(){};
};
// TEST( is_range_detect, is_range_test )
// {
//     std::vector<int> v{0, 2, 3, 4};
//     auto&& testv = is_range_f(std::vector<int>());
//     auto&& testt = is_range_f( v | ba::transformed(
//         [](auto&& o) { return o + 3; }
//     ) );
//     auto&& testq = is_range_v<QQ>;
//     auto&& testi = is_range_v<int>;
//     std::cout << testv << std::endl;
//     std::cout << testt << std::endl;
//     std::cout << testq << std::endl;
//     std::cout << testi << std::endl;
//     EXPECT_TRUE(testv);
//     EXPECT_TRUE(testt);
//     EXPECT_TRUE(testq);
//     EXPECT_FALSE(testi);
// }
template<class T>
void print_type()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}
void nested_print(int i )
{
    std::cout << i << std::endl;
}
template<class RNG>
void nested_print( RNG&& rng )
{
    for(auto&& v : rng )
    {
        nested_print(std::forward<decltype(v)>(v));
    }
}
struct Eval{};
template<class T1, class T2, class T3> 
auto operator|(::SolidRange<T1, T2, T3>&& sr, Eval q)
{
    return sr.orng;
}
// TEST( state_flatten, basic_test )
int main()
{
    std::vector<std::vector<int>> simple{
          { 0, 2, 3, 4 }
        , { 2, 5, 2, 6}
    };
    std::vector<int>v{ 0, 2, 4, 6 };
    auto&& a( 
        std::vector<int>{ 0, 2, 4, 6 }
            | glue(ba::transformed(
                []( auto&& i )
                {
                    return std::vector<int>(i,i);
                }
            )) 
    )
    ;
    auto&& b = a | ca::state_flattened();
    // print_type<decltype(b)>();
    nested_print(a);
    std::cout << "=========" << std::endl;
    // for( auto&& i : simple | ca::state_flattened() )
    //     std::cout << i << std::endl;
    for( auto&& i : b)
    {
        std::cout << i << std::endl;
    }
    ;
}
