#pragma once
#include <CPT/utility/thread_data_table.hpp>
#include <chrono>
#include <vector>

namespace cpt {
namespace utility {

template<template<class V> class STORAGE, class LABEL, class T>
struct SliceSamplingTable : public STORAGE<std::map<LABEL, T>>
{
    // std::thread realtime_reporter;
    // bool realtime_report_on {false};
    struct Holder
    {
        bool valid;
        std::chrono::steady_clock::time_point start_point;
        T& data;
        Holder( T& _data  )
        : start_point ( std::chrono::steady_clock::now() )
        , data ( _data )
        , valid( true  )
        {}
        void release()
        {
            if ( valid )
            {
                auto now ( std::chrono::steady_clock::now() );
                auto du = std::chrono::duration_cast<std::chrono::microseconds>( now - start_point).count();
                data.update(du);
                valid = false;
            }
        }
        ~Holder()
        {
            release();
        }

    };
    SliceSamplingTable() = default;
    template<class... ARGS>
    decltype(auto) get_holder( ARGS&&... args )
    {
        return Holder(this->get()[
            LABEL(std::forward<ARGS>(args)...)
        ]);
    }
    auto get_holder( const LABEL& label )
    {
        return Holder(this->get()[label]);
    }
    template<class LOG>
    void report( LOG& logger )
    {
        T::report( this->look_storage(), logger );
    }
    // template<class LOG>
    // void realtime_report( LOG&& logger, const std::size_t& tick_sec)
    // {
    //     realtime_report_on = true;
    //     realtime_reporter = [&logger, this, tick_sec] ( )
    //     {
    //         while ( realtime_report_on )
    //         {
    //             report ( logger );
    //             std::this_thread::sleep_for(std::chrono::seconds(tick_sec));
    //         }
    //         report( logger );
    //     };
    // };
    // ~SliceSamplingTable()
    // {
    //     if ( realtime_report_on )
    //     {
    //         realtime_report_on = false;
    //         realtime_reporter.join();
    //     }

    // }
};

}}
