#include <mlpack/core.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/components/data_loader.hpp>
#include <CCD/utility/language.hpp>
#include "shared.hpp"
#include <CPT/application/pipeline_builder/main.hpp>
namespace cap = cpt::application::pipeline_builder;
TEST(component_test, target_sketch_estimation_test)
{
    /* prepare option */
    auto in_path = nucleona::language::sdir(__FILE__) 
        #ifdef NEW_DATA_POOL
        + "/target_sketch_estimation_test/in_d2.json"s
        #else
        + "/target_sketch_estimation_test/in.json"s
        #endif
    ;
    Option option{ 
        std::ifstream( std::move( in_path ) ) 
        , std::stringstream() 
    };

    auto&& pipeline_builder ( cap::make( option ) );
    pipeline_builder();

    /* 2. test component initialize */
    std::cout << option.result_schema_stream_.str() << std::endl;
    
}
