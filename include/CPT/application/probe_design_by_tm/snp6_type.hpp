#pragma once
#include <CCD/analyzer/probe_tm.hpp>
#include <CPT/application/probe_design_by_tm/data_pool.hpp>
#include <CPT/application/probe_design_by_tm/probe_check.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

class Snp6Type
{
  public:

    cpt::application::probe_design_by_tm::DataPool& db;

    template< class DP >
    Snp6Type( DP& dp )
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

            for( size_t allele = 0; allele < std::get<5>( db.beds[ id ] ).size(); ++allele )
            {
                std::function< void( void ) > task = [ this, id, allele, chr, strand, &tsc ]()
                {
                    std::map< double, ProbeTupleType > res_map;
                    std::string sequence;

                    size_t start;
                    size_t len = db.min_len;
                    size_t sweep_len = 0;

                    while( !( len > db.max_len ))
                    {
                        start = std::get<1>( db.beds[ id ] ) - db.no_snp_len - sweep_len;
                        sequence = get_sequence( chr, start, std::get<1>( db.beds[ id ] ), std::get<5>( db.beds[ id ] )[ allele ], len, strand );

                        if( start - std::get<1>( db.beds[ id ] ) + len -1 > db.no_snp_len )
                        {
                            sweep_len++;
                        }
                        else
                        {
                            len++;
                            sweep_len = 0;
                        }

                        if( sequence.length() == 0 )
                        {
                            continue;
                        }

                        probe_tm tm( sequence, db.K_mM, db.divalent, db.dntp, db.DNA_nM );
                        res_map.emplace( tm.tmp, std::make_tuple(
                            chr, start, start + sequence.length(), strand, sequence, 0, 0, false, false, 0, 0, false, false
                        ));
                    }

                    bool is_tm_probe = false;
                    std::pair< double, ProbeTupleType > last_pair( std::make_pair( 0, ProbeTupleType() ));
                    std::pair< double, ProbeTupleType > this_pair( std::make_pair( 0, ProbeTupleType() ));

                    if( db.is_probecheck )
                    {
                        ProbeCheck probe_check( db, tsc );
                        probe_check.align_check( res_map );
                        probe_check.anno_check_snp6( id, allele, res_map );

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
                            db.res_vec[ id ][ allele ] = this_pair;
                            break;
                        }
                    }

                    if( !is_tm_probe )
                    {
                        db.res_vec[ id ][ allele ] = this_pair;
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

    std::string get_sequence( const std::string& chr, const size_t& start, const size_t& snp_site, const std::string& allel, const size_t& length, const std::string& strand )
    {
        std::string res;

        switch( strand.at(0) )
        {
            case '+' :
                res = sequence_is_valid(
                    db.chr_table[ chr ].substr( start, snp_site - start ) + allel + db.chr_table[ chr ].substr( snp_site +1, ( start -1 + length ) - snp_site )
                );
                break;

            case '-' :
                res = sequence_is_valid( reverse_complement(
                    db.chr_table[ chr ].substr( start, snp_site - start ) + reverse_complement( allel ) + db.chr_table[ chr ].substr( snp_site +1, ( start -1 + length ) - snp_site )
                ));
                break;

            default : break;
        }

        return res;
    }
};

} // probe_design_by_tm
} // application
} // cpt
