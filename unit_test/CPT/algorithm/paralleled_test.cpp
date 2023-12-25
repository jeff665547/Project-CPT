#include <CPT/algorithm/paralleled.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/range.hpp>
#include <mutex>
#include <CCD/para_thread_pool/para_thread_pool.hpp>
RANGE_NAMESPACE_SHORTCUT
TEST ( paralleled_test, basic_test )
{
    ParaThreadPool tp(1);
    std::mutex mux;
    cu::irange_0(1000)
    | ba::transformed(
        []( auto i )
        {
            return i * 2;
        }
    )
    | ba::transformed(
        [&mux]( auto i )
        {
            std::lock_guard<std::mutex> lock ( mux );
            std::cout << i << std::endl;
            return 0;
        }
    )
    | ca::parallel_eval(tp, 27)
    ;

}
