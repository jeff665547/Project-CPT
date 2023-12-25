#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/engine/data_pool/monitor.hpp>
#include <CCD/para_thread_pool/para_thread_pool.hpp>

TEST( Monitor, general_use )
{
    std::ofstream mointor_outfile;
    auto out = cpt::engine::data_pool::make_monitor( cpt::msg, mointor_outfile );

    out.set_output_dir( "./general_" );

    size_t i_total = 2000000;
    size_t j_total = 4000000;

    out.set_monitor( "TEST", 5 );

    out.set_monitor( "i loop", i_total );
    out.set_monitor( "j loop", j_total );

    out.log( "TEST", "Test start" );

    for( size_t i = 0; i < i_total/2; ++i )
    {
        out.log( "i loop", "First half of 200K" );
    }

    out.log( "TEST", "Testing..." );

    for( size_t j = 0; j < j_total/2; ++j )
    {
        out.log( "j loop", "First half of 400K" );
    }

    out.log( "TEST", "Testing..." );

    for( size_t k = 0; k < i_total/2; ++k )
    {
        out.log( "i loop", "Second half of 200K" );
    }

    out.log( "TEST", "Testing..." );

    for( size_t l = 0; l < j_total/2; ++l )
    {
        out.log( "j loop", "Second half of 400K" );
    }

    out.clear_done();

    out.log( "TEST", "Test is done" );
}

TEST( Monitor, para_use )
{
    std::ofstream mointor_outfile;
    auto out = cpt::engine::data_pool::make_monitor( cpt::msg, mointor_outfile );

    out.set_output_dir( "./para_" );
    out.set_monitor( "TEST", 3 );

    out.log( "TEST", "Test start" );

    size_t total = 10;

    std::mutex mutex;
    ParaThreadPool parallel_pool( 2 );
    std::vector< size_t > jobs;

    out.set_monitor( "ParaLoop", 2, true );

    for( size_t i = 0; i < total; ++i )
    {
        jobs.push_back( i );

        if( jobs.size() < 5 )
        {
            if( i != total -1 )
            {
                continue;
            }
        }

        parallel_pool.job_post( [ jobs, &mutex, &out ] ()
        {
            for( auto& job : jobs )
            {
                out.thread_monitor_start( "ParaLoop" );
                std::this_thread::sleep_for( 2s );
                out.thread_monitor_end( "ParaLoop" );
            }

            {
                std::lock_guard< std::mutex > lock( mutex );
                out.log( "ParaLoop", " ... ", true );
            }
        });

        jobs.clear();
    }

    out.log( "TEST", "Flushing..." );
    parallel_pool.flush_pool();
    out.log( "TEST", "Test is done" );
}
