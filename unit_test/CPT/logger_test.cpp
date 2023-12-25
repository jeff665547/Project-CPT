#include <Nucleona/app/cli/gtest.hpp>
#define LOGGER_OPEN_CHANNEL 0,1,2,4
#include <CPT/logger.hpp>
TEST(logger, general_use)
{
    cpt::logout0 << "hello0" << std::endl;
    cpt::logout1 << "hello1" << std::endl;
    cpt::logout2 << "hello2" << std::endl;
    cpt::logout3 << "this will not print" << std::endl;
    cpt::logout4 << "hello4" << std::endl;
}
TEST(logger, customize)
{
    typedef cpt::logger::LoggerEngine<0,1,2> LoggerGen;
    auto log0 = LoggerGen::create<0>(std::cout);
    auto log1 = LoggerGen::create<1>(std::cout);
    auto log3 = LoggerGen::create<3>(std::cout);
    log0 << "hello0" << std::endl;
    log1 << "hello1" << std::endl;
    log3 << "this will not print" << std::endl;
}
