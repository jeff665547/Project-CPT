#include <CPT/utility/thread_task_time_reporter.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <CCD/para_thread_pool/para_thread_pool.hpp>
TEST( thread_task_table, duration )
{
    using namespace std::chrono_literals;
    cpt::utility::TickReport reporter ( 1 );
    cpt::utility::ThreadTaskTimeReporter rp(std::cout, reporter);
    rp.start();
    for ( auto i = 0; i < 3; i++ )
    {
        auto&& rh = rp.get_holder("test1", cpt::utility::ThreadDurationLabel::iterative_mode);
        std::this_thread::sleep_for(3s);
    }
    rp.stop();
}


TEST( thread_task_table, multithread_du )
{
    using namespace std::chrono_literals;
    ParaThreadPool tp(3);
    cpt::utility::TickReport reporter(1);
    cpt::utility::ThreadTaskTimeReporter rp(std::cout, reporter);
    rp.start();
    for ( auto i = 0; i < 3; i++ )
    {
        tp.JobPost([&rp]()
        {
            auto&& rh = rp.get_holder("test2", cpt::utility::ThreadDurationLabel::parallel_mode);
            std::this_thread::sleep_for(3s);
            rh.release();
        });
    }
    tp.FlushPool();
    rp.stop();
}

