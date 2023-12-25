
#include <mlpack/core.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/components/data_loader.hpp>
#include <CCD/utility/language.hpp>
#include "shared.hpp"
#include <CPT/application/pipeline_builder/main.hpp>
#include <CPT/format/json.hpp>
using namespace std::string_literals;
template<class O>
void run( O&& o )
{
    exit(o());
}
namespace cap = cpt::application::pipeline_builder;
auto get_in_path( 
    const std::string& fname 
)
{
    return __SDIR__ + "/allele_summarization_test/"s + fname;
}
TEST(component_test, allele_summarization_test)
{
    auto in_path = get_in_path(
        #ifdef NEW_DATA_POOL
        "in_t2.json"s
        #else
        "in.json"s
        #endif
    );
    /* prepare option */
    Option option{ 
        std::ifstream( std::move( in_path ) ) 
        , std::stringstream() 
    };
    auto&& pipeline_builder ( cap::make( option ) );

    // EXPECT_EXIT( run ( pipeline_builder ), ::testing::ExitedWithCode(0), ".*" );
    run ( pipeline_builder );
}
// TEST(component_test, allele_summarization_test)
// {
//     auto in_path = get_in_path(
//         #ifdef NEW_DATA_POOL
//         "in_t2.json"s
//         #else
//         "in.json"s
//         #endif
//     );
//     /* prepare option */
//     Option option{ 
//         std::ifstream( std::move( in_path ) ) 
//         , std::stringstream() 
//     };
//     auto&& pipeline_builder ( cap::make( option ) );
// 
//     EXPECT_EXIT( run ( pipeline_builder ), ::testing::ExitedWithCode(0), ".*" );
// }
// TODO FIXME 
// TEST(component_test, allele_summarization_fail_test)
// {
//     /* prepare option */
//     auto in_path = get_in_path( 
//         #ifdef NEW_DATA_POOL
//         "in_d2_fail.json"s
//         #else
//         "in_fail.json"s
//         #endif
//     );
//     Option option{ 
//         std::ifstream( std::move( in_path ) ) 
//         , std::stringstream() 
//     };
//     auto&& pipeline_builder ( cap::make( option ) );
//     EXPECT_DEATH( pipeline_builder(), ".*" );
// }
