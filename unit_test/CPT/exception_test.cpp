#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
#include <Nucleona/app/cli/gtest.hpp>
TEST( exception, cpt_dump_test )
{
    try 
    {
        throw cpt::Exception("test exception"); 
    }
    catch ( const cpt::Exception& e )
    {
        e.json_dump(std::cout);
    }
}
TEST( exception, std_dump_test )
{
    try 
    {
        throw std::logic_error("test std logic_error");
    }
    catch ( const std::exception& e )
    {
        cpt::exception::std_exception_json_dump(e, std::cout);
    }
}
