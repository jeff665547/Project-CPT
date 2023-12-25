#pragma once
#include <utility>
#include <CPT/utility/thread_data_table.hpp>
#include <iostream>
#include <iomanip>
namespace cpt {
namespace utility {

struct ThreadDurationMode
{
    virtual const std::string label_type() const = 0;
    virtual const double task_time(
          std::size_t du_sum
        , std::size_t t_num
    ) const = 0;
};
using ThreadDurationLabelBase = std::pair<
      std::string
    , const ThreadDurationMode*
>;
struct ThreadDurationLabel : public ThreadDurationLabelBase
{
    using Base = ThreadDurationLabelBase;
    static struct : public ThreadDurationMode
    {
        virtual const std::string label_type() const override
        { return "parallel"; }
        virtual const double task_time(
              std::size_t du_sum
            , std::size_t t_num
        ) const override
        { 
            return du_sum / (double) t_num;
        }
    } parallel_mode;
    static struct : public ThreadDurationMode
    {
        virtual const std::string label_type() const override
        { return "iterative"; }
        virtual const double task_time(
              std::size_t du_sum
            , std::size_t t_num
        ) const override
        { 
            return (double)du_sum;
        }
    } iterative_mode;

    ThreadDurationLabel ( const std::string& str, const ThreadDurationMode& mode )
    : Base ( str, &mode )
    {}
    bool operator<( const ThreadDurationLabel& o )
    {
        if ( Base::first == o.first )
            return ((int64_t&)Base::second) < ((int64_t&)o.second);
        else return Base::first < o.first;
    }
};
struct ThreadDuration
{
    double du_sum{0};
    void update( std::size_t du )
    {
        du_sum += ( du / 1000.0 );
    }
    template<class THREAD_TAB, class LOG>
    static void report(THREAD_TAB& tdt, LOG& logger )
    {
        std::map<
              ThreadDurationLabel
            , std::tuple<
                  std::size_t   /* n_threads        */
                , double        /* thread_du_sum    */
            >
        > lab_tab;
        for(auto&& t_lt : tdt )
        {
            for ( auto&& lab_du : t_lt.second )
            {
                auto& n_threads = std::get<0>(lab_tab[lab_du.first]);
                auto& tdu_sum   = std::get<1>(lab_tab[lab_du.first]);
                n_threads ++; 
                tdu_sum += lab_du.second.du_sum;
            }
        }
        logger 
            << std::setw(70) << "[label type]"
            << std::setw(15) << "task_time" 
            << std::setw(15) << "thread_num" 
            << std::setw(15) << "duration_sum" 
            << std::endl;
        for ( auto&& lab_d : lab_tab )
        {
            auto& du_sum = std::get<1>(lab_d.second );
            auto& t_num  = std::get<0>(lab_d.second );
            logger 
                << std::setw(70) << "[ " + lab_d.first.first + " " + lab_d.first.second->label_type() + " ]"
                << std::setw(15) 
                    << lab_d.first.second->task_time(du_sum, t_num)
                << std::setw(15) << t_num
                << std::setw(15) << du_sum 
                << std::endl;
        }
    }
};

}}
