#pragma once
#include <CCD/analyzer/probe_tm.hpp>
#include <CPT/application/probe_design_by_tm/data_pool.hpp>
#include <CPT/application/probe_design_by_tm/probe_check.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

class AxiomType
{
  public:

    cpt::application::probe_design_by_tm::DataPool& db;
    
    template< class DP >
    AxiomType( DP& dp )
    : db( dp )
    {}

    void operator()()
    {
        ThreadSafeCounter tsc;
        ParaThreadPool thread_pool( db.thread_num );
        std::vector< std::function< void( void ) >> tasks;

        for( size_t id = 0; id < db.beds.size(); ++id )
        {
            std::string& chr    = std::get<0>( db.beds[ id ]);
            std::string& strand = std::get<3>( db.beds[ id ]);

            for( size_t fwbw = 0; fwbw < 2; ++fwbw )
            {
                std::function< void( void ) > task = [ this, id, fwbw, chr, strand, &tsc ]()
                {
                    std::map< double, ProbeTupleType > res_map;
                    std::string sequence;

                    size_t start;
                    size_t len = db.min_len;
                    size_t sweep_len = 0;

                    while( !( len > db.max_len ))
                    {
                        switch( fwbw )
                        {
                            case 0 :
                                start = std::get<1>( db.beds[ id ] ) - len;
                                sequence = get_sequence( chr, start, len );
                                break;

                            case 1 :
                                start = std::get<2>( db.beds[ id ] );
                                sequence = reverse_complement( get_sequence( chr, start, len ));
                                break;
                        }

                        probe_tm tm( sequence, db.K_mM, db.divalent, db.dntp, db.DNA_nM );
                        res_map.emplace( tm.tmp, std::make_tuple(
                            chr, start, start + sequence.length(), strand, sequence, 0, 0, false, false, 0, 0, false, false
                        ));

                        len++;
                    }

                    bool is_tm_probe = false;
                    std::pair< double, ProbeTupleType > last_pair( std::make_pair( 0, ProbeTupleType() ));
                    std::pair< double, ProbeTupleType > this_pair( std::make_pair( 0, ProbeTupleType() ));

                    if( db.is_probecheck )
                    {
                        ProbeCheck probe_check( db, tsc );
                        probe_check.align_check( res_map );
                        probe_check.anno_check_axiom( id, fwbw, res_map );

                        std::map< double, ProbeTupleType > res_temp;

                        for( auto& probe : res_map )
                        {
                            if( std::get<8>( probe.second ))
                            {
                                res_temp.emplace( probe );
                            }
                        }

                        if( !res_temp.empty() )
                        {
                            res_map = res_temp;
                        }
                    }

                    for( auto& tmp : res_map )
                    {
                        this_pair = tmp;

                        if( is_tm( db.tm, last_pair, this_pair ))
                        {
                            is_tm_probe = true;
                            db.res_vec[ id ][ fwbw ] = this_pair;
                            break;
                        }
                    }

                    if( !is_tm_probe )
                    {
                        db.res_vec[ id ][ fwbw ] = this_pair;
                    }
                };

                tasks.push_back( task );
            }

            if( tasks.size() >= db.task_num )
            {
                task_post( thread_pool, tasks );
            }
        }

        task_post( thread_pool, tasks );
        thread_pool.flush_pool();
    }

    void task_post( auto& thread_pool, auto& tasks )
    {
        std::function< void( void ) > task = [ tasks ]()
        {
            for( auto& task : tasks )
            {
                task();
            }
        };

        thread_pool.job_post( task );
        tasks.clear();
    }

    std::string get_sequence( const std::string& chr, const size_t& start, const size_t& length )
    {
        return sequence_is_valid( db.chr_table[ chr ].substr( start, length ));
    }
};

} // probe_design_by_tm
} // application
} // cpt
