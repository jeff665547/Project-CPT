#pragma once
#include <CPT/utility/slice_sampling_table.hpp>
#include <CPT/utility/thread_duration.hpp>
#include <CPT/utility/tick_report.hpp>
#include <Nucleona/language.hpp>
namespace cpt {
namespace utility {
using ThreadTaskTimeReporterBase = SliceSamplingTable<
      ThreadDataTable
    , ThreadDurationLabel
    , ThreadDuration
>;
struct ThreadTaskTimeReporter 
: public ThreadTaskTimeReporterBase
{
    using Base = ThreadTaskTimeReporterBase;
    TickReport& tick_report;
    std::size_t task_id;
    template<class LOG>
    ThreadTaskTimeReporter(LOG& logger, TickReport& _tick_report)
    : Base () 
    , tick_report( _tick_report )
    {
        task_id = tick_report.add_event( [this,&logger]()
        {
            this->report(logger); 
        });
    }
    DEFAULT_MOVE(ThreadTaskTimeReporter);
    DISABLE_COPY(ThreadTaskTimeReporter);
    void start()
    {
        tick_report.tick_start();
    }
    void stop()
    {
        std::this_thread::sleep_for(std::chrono::seconds(tick_report.tick_sec * 2));
        tick_report.remove_event(task_id);
    }
    ~ThreadTaskTimeReporter()
    {
        stop();
    }
};
}}
