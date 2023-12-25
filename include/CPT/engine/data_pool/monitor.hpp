#pragma once
#define CPT_MONITOR_TGUI

#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <map>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/forward.hpp>
#include <CPT/const_tags.hpp>
#include <CPT/format/json.hpp>
#include <mutex>
#include <thread>

namespace cpt {
namespace engine {
namespace data_pool {

struct LogContent
{
    size_t current;
    size_t total;
    std::string msg;
    std::string percentage;
    std::string start_time;
    std::string end_time;
    std::chrono::time_point< std::chrono::system_clock > time_point;
    std::string est_time;
    std::map<
          std::thread::id
        , std::pair<
              std::chrono::time_point< std::chrono::system_clock >
            , double
        >
    > para_map;

    LogContent()
        : current( 0 )
        , total( 0 )
        , msg( "" )
        , percentage( "" )
        , start_time( "" )
        , end_time( "" )
        , time_point( std::chrono::time_point< std::chrono::system_clock >() )
        , est_time( "" )
        , para_map()
    {
    }

    LogContent( size_t& total_item )
        : current( 0 )
        , total( total_item )
        , msg( "" )
        , percentage( "" )
        , start_time( "" )
        , end_time( "" )
        , time_point( std::chrono::time_point< std::chrono::system_clock >() )
        , est_time( "" )
        , para_map()
    {
    }
};

std::mutex mutex;

template< class __OUT >//, class PARALLEL = cpt::NoParallel >
class Monitor
{
    std::ofstream& output_;

  public:

    __OUT& out;

    std::vector< std::string > tag_order;
    std::chrono::time_point< std::chrono::system_clock > pipeline_start_time;

    std::map< std::string, LogContent > log_table;

    Monitor( __OUT& log, std::ofstream& output )
        : output_( output )
        , out( log )
        , pipeline_start_time( get_time_point() )
    {
    }

    void set_output_dir( std::string path )
    {
        path += "monitor.log";
        output_.open( path );
    }

    std::string get_now_time()
    {
        std::time_t systime( std::time( nullptr ));
        std::tm now = *std::localtime( &systime );

        std::stringstream ss;
        ss << std::put_time( &now, "%F %T" );

        return ss.str();
    }

    std::chrono::time_point< std::chrono::system_clock > get_time_point()
    {
        return std::chrono::time_point< std::chrono::system_clock >( std::chrono::system_clock::now() );
    }

    std::string load_percentage( size_t& current, size_t& total )
    {
        size_t nu( current * 100 / total );
        std::string num = std::to_string( nu );

        if( num.length() == 1 )
        {
            return " " + num + "% ";
        }
        else if( num.length() == 3 )
        {
            return num + "%";
        }

        return " " + num + "%";
    }

    std::string json_load_percentage( size_t& current, size_t& total )
    {
        size_t nu( current * 100 / total );
        std::string num = std::to_string( nu );

        return num + "%";
    }

    std::string count( const double& est )
    {
        std::string st = std::to_string( est );

        std::vector< std::string > stv;
        boost::iter_split( stv, st, boost::algorithm::first_finder( "." ));

        size_t i = 14 - stv[0].length();

        std::stringstream sss;
        sss << std::setiosflags( std::ios::fixed ) << std::setprecision( i ) << est;

        return sss.str();
    }

    void clear_done()
    {
        std::vector< std::string > temp;

        for( auto& log_tag : tag_order )
        {
            std::map< std::string, LogContent >::iterator log_table_it;
            log_table_it = log_table.find( log_tag );
            auto& time   = log_table_it->second.est_time;

            if( time == "" )
            {
                temp.push_back( log_tag );
                continue;
            }

            log_table.erase( log_table_it );
        }

        temp.swap( tag_order );
    }

    void clear_screen()
    {
        int __attribute__((unused)) unused; 
        unused = std::system( "clear" );
    }

    void drop_tag( const std::string& tag )
    {
        std::vector< std::string > temp;

        for( auto& log_tag : tag_order )
        {
            if( tag == log_tag )
            {
                continue;
            }

            temp.push_back( log_tag );
        }

        temp.swap( tag_order );
    }

    void set_monitor( std::string&& tag, size_t total_item, bool is_para = false )
    {
        if( !output_.is_open() )
        {
            throw std::runtime_error( "The output file of monitor is not set\n" );
        }

        std::map< std::string, LogContent >::iterator log_table_it;
        log_table_it = log_table.find( tag );

        if( log_table_it != log_table.end() )
        {
            auto& time = log_table_it->second.est_time;

            if( time == "" )
            {
                throw std::runtime_error( "Error for duplicate tag and is not ending at  \"" + tag + "\" in the log_table of the Monitor\n" );
            }

            drop_tag( log_table_it->first );
            log_table.erase( log_table_it );
        }

        tag_order.push_back( tag );

        log_table.emplace( tag, LogContent( total_item )); 

        if( is_para )
        {
            log_table[ tag ].start_time = get_now_time();
        }
    }

    void thread_monitor_start( const std::string& tag )
    {
        auto start = get_time_point();

        std::thread::id this_id( std::this_thread::get_id() );
        std::map< std::string, LogContent >::iterator log_table_it = log_table.find( tag );

        if( log_table_it == log_table.end() )
        {
            throw std::runtime_error( "Error for tag not found \"" + tag + "\" in the log_table of the Monitor\n" );
        }

        if( log_table_it->second.para_map.find( this_id ) == log_table_it->second.para_map.end() )
        {
            std::lock_guard< std::mutex > lock( mutex );
            log_table_it->second.para_map[ this_id ] = std::make_pair( start, 0.0 );
        }
        else
        {
            log_table_it->second.para_map[ this_id ].first = start;
        }
    }

    void thread_monitor_end( const std::string& tag )
    {
        auto end = get_time_point();

        std::thread::id this_id( std::this_thread::get_id() );
        std::map< std::string, LogContent >::iterator log_table_it = log_table.find( tag );

        log_table_it->second.para_map[ this_id ].second += std::chrono::duration< double >( end - log_table_it->second.para_map[ this_id ].first ).count();
    }

    void thread_monitor_sum( const std::string& tag )
    {
        std::map< std::string, LogContent >::iterator log_table_it = log_table.find( tag );

        double sum = 0.0;
        size_t thread_count = log_table_it->second.para_map.size();

        for( auto& para_map : log_table_it->second.para_map )
        {
            sum += para_map.second.second;
        }

        sum = sum / thread_count;
        log_table_it->second.est_time = count( sum );
    }

    void log( std::string&& tag, std::string&& new_msg, bool is_para = false )
    {
        std::string output_tag = "";

        std::map< std::string, LogContent >::iterator log_table_it;
        log_table_it = log_table.find( tag );

        if( log_table_it == log_table.end() )
        {
            throw std::runtime_error( "Error for tag not found \"" + tag + "\" in the log_table of the Monitor\n" );
        }

        auto& current = log_table_it->second.current;
        auto& total   = log_table_it->second.total;
        auto& msg     = log_table_it->second.msg;
        auto& percent = log_table_it->second.percentage;
        auto& st_time = log_table_it->second.start_time;
        auto& ed_time = log_table_it->second.end_time;
        auto& time_pt = log_table_it->second.time_point;
        auto& time    = log_table_it->second.est_time;

        if( current == 0 && !is_para )
        {
            st_time = get_now_time();
            time_pt = get_time_point();
        }

        current++;

        std::string per = load_percentage( current, total );

        if( per == percent )
        {
            return;
        }

        if( current > total )
        {
            return;
        }

        percent = per;
        msg = new_msg;

        if( current == total )
        {
            ed_time = get_now_time();
            output_tag = log_table_it->first;

            if( is_para )
            {
                thread_monitor_sum( tag );
            }
            else
            {
                time = count( std::chrono::duration< double >( get_time_point() - time_pt ).count() );
            }

            file_log( log_table_it );
        }

#ifdef CPT_MONITOR_GUI

        gui_log();

#endif
#ifdef CPT_MONITOR_TGUI

        t_gui_log();

#endif

    }

    void file_log( std::map< std::string, LogContent >::iterator log_it )
    {
        auto& tag = log_it->first;
        auto& msg     = log_it->second.msg;
        auto& st_time = log_it->second.start_time;
        auto& ed_time = log_it->second.end_time;
        auto& time    = log_it->second.est_time;

        output_ << st_time << "\t" << ed_time << "\t" << time << "\t" << tag << "\t" << msg << "\n";
    }

    void gui_log()
    {
        // bpt::ptree json;
        auto json ( cpt::format::make_json() );
        std::stringstream sss;

        std::map< std::string, LogContent >::iterator log_table_it;

        for( auto& log_tag : tag_order )
        {
            log_table_it = log_table.find( log_tag );
            auto subject = json.create_child(log_table_it->first);

            auto& current = log_table_it->second.current;
            auto& total   = log_table_it->second.total;
            auto& msg     = log_table_it->second.msg;
            auto& st_time = log_table_it->second.start_time;
            auto& time    = log_table_it->second.est_time;

            if( st_time == "" )
            {
                continue;
            }

            // bpt::ptree node;
            // bpt::ptree msg_node;
            // bpt::ptree status_node;
            // bpt::ptree time_node;

            if( time != "" )
            {
                // status_node.put( "status", "OK" );
                // time_node.put(  "time", time );
                subject.add("status", "OK");
                subject.add("time", time);
            }
            else
            {
                subject.add("status", json_load_percentage( current, total ));
                subject.add("time", st_time);
                // status_node.put( "status", json_load_percentage( current, total ));
                // time_node.put( "time", st_time );
            }

            // msg_node.put( "msg", msg );
            subject.add("msg", msg);

            // node.push_back( std::make_pair( "", msg_node ));
            // node.push_back( std::make_pair( "", status_node ));
            // node.push_back( std::make_pair( "", time_node ));

            // json.add_child( log_table_it->first, node );
        }

        json.dump(sss, false);
        // bpt::write_json( sss, json, false );
        out << sss.str();
        out.flush();
    }

    void t_gui_log()
    {
        clear_screen();
        std::map< std::string, LogContent >::iterator log_table_it;

        for( auto& log_tag : tag_order )
        {
            log_table_it = log_table.find( log_tag );

            auto& msg     = log_table_it->second.msg;
            auto& percent = log_table_it->second.percentage;
            auto& st_time = log_table_it->second.start_time;
            auto& time    = log_table_it->second.est_time;

            if( st_time == "" )
            {
                continue;
            }


            if( time != "" )
            {
                out << "[\033[1;32m OK \033[0m]";
                out << "[\033[1;33m " << time << " sec \033[0m] ";
            }
            else
            {
                out << "[\033[1;36m" << percent << "\033[0m]";
                out << "[\033[1;33m " << st_time << " \033[0m] ";
            }

            out << log_table_it->first << " - " << msg << std::endl;
        }
    }

    virtual ~Monitor()
    {
        auto pipeline_end_time = get_time_point();
        std::chrono::duration< double > est( pipeline_end_time - pipeline_start_time );
        auto time = count( est.count() );

        output_ << "Pipeline Total Cost Time: " << time <<" sec\n";
    }
};

template< class __OUT >//, class PARALLEL = cpt::NoParallel >
auto make_monitor( __OUT& out, std::ofstream& mointor_outfile )
{
    return Monitor< __OUT >( out, mointor_outfile );
}

} // end of namespace data_pool
} // end of namespace engine
} // cpt
