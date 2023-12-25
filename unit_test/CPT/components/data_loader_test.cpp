#include <Nucleona/app/cli/gtest.hpp>
#include <mlpack/core.hpp>
#include <CPT/components/data_loader.hpp>
#include <CCD/utility/language.hpp>
#include "shared.hpp"
#include <CPT/application/pipeline_builder/main.hpp>
#include <string>
#include <boost/lexical_cast.hpp>
namespace cap = cpt::application::pipeline_builder;
using namespace std::string_literals;
// TEST(component_test, data_loader)
// {
//     /* prepare option */
//     auto in_path = sdir(__FILE__)
//         #ifdef NEW_DATA_POOL
//         + "/data_loader_test/in_d2.json"s
//         #else
//         + "/data_loader_test/in.json"s
//         #endif
//     ;
//     Option option{ 
//         std::ifstream( std::move( in_path ) ) 
//         , std::stringstream() 
//     };
// 
//     auto&& pipeline_builder ( cap::make( option ) );
//     pipeline_builder();
// 
//     /* 2. test component initialize */
//     std::cout << option.result_schema_stream_.str() << std::endl;
//     
// }
TEST(component_test, data_loader_cen)
{
    /* prepare option */
    auto in_path = nucleona::language::sdir(__FILE__)
        #ifdef NEW_DATA_POOL
        + "/data_loader_test/in_d3.json"s
        #else
        + "/data_loader_test/in.json"s
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
