#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <Nucleona/language.hpp>
namespace cpt {
namespace utility {
struct TickReport
{
  private:
    std::vector<
        std::function<void(void)>
    > events;
    std::unique_ptr<std::mutex> events_mux{new std::mutex()};
  public:
    bool flag;
    std::unique_ptr<std::thread> worker;
    const std::size_t tick_sec;
    TickReport( const std::size_t& tick )
    : flag      ( false )
    , tick_sec  ( tick )
    {}
    DEFAULT_MOVE(TickReport)
    DISABLE_COPY(TickReport)
    template<class F>
    auto add_event( F&& f )
    {
        std::lock_guard<std::mutex> lock(*events_mux);
        events.emplace_back( std::forward<F>(f) );
        return events.size() - 1;
    }
    void event_loop()
    {
        std::lock_guard<std::mutex> lock(*events_mux);
        for ( auto&& e : events ){ e(); }
    }
    void remove_event( std::size_t id )
    {
        std::lock_guard<std::mutex> lock(*events_mux);
        events.erase( events.begin() + id );
    }
    void tick_start()
    {
        flag = true; 
        worker.reset(new std::thread([this] ()
        {
            while(flag)
            {
                event_loop();
                std::this_thread::sleep_for(std::chrono::seconds(tick_sec));
            }
            event_loop();
        }));
    }
    void tick_stop()
    {
        flag = false;
        if ( worker->joinable() )
            worker->join();
    }
    ~TickReport()
    {
        tick_stop();
    }
};
}}
