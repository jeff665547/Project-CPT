#include <CPT/application/pipeline_builder/main.hpp>
#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
int main ( int argc, const char* argv[] )
{
    cpt::application::pipeline_builder::OptionParser option_parser(argc, argv);
//    try
//    {
        auto&& pipeline_builder ( cpt::application::pipeline_builder::make( option_parser ) );
        pipeline_builder();
        return 0;
//    }
//    catch ( const cpt::Exception& e )
//    {
//        e.json_dump ( option_parser.result_schema_stream_ );
//    }
//    catch ( const std::exception& e )
//    {
//        cpt::exception::std_exception_json_dump ( e, option_parser.result_schema_stream_ );
//    }
}
