#include <Nucleona/app/cli/gtest.hpp>
#include <CCD/utility/language.hpp>
#include <CPT/application/pipeline_builder/main.hpp>
#include "../shared.hpp"
namespace cap = cpt::application::pipeline_builder;
using namespace std::string_literals;
auto get_in_path()
{
    return
        nucleona::language::sdir(__FILE__)
        #ifdef NEW_DATA_POOL
        + "/grand_model_training_test/in_d2.json"s
        #else
        + "/grand_model_training_test/in.json"s
        #endif
    ;
}
void run(bool flag)
{
    auto in_path = get_in_path();
    Option option{ 
          std::ifstream( std::move( in_path ) ) 
        , std::stringstream() 
    };

    auto&& pipeline_builder ( cap::make( option ) );
    int code = pipeline_builder();

    /* 2. test component initialize */
    std::cout << option.result_schema_stream_.str() << std::endl;
    if ( flag )
        exit( code );
}
TEST(component_test, grand_model_training)
{
    /* prepare option */
    // run(false);
    EXPECT_EXIT( run(true), ::testing::ExitedWithCode(0), ".*" );

}
