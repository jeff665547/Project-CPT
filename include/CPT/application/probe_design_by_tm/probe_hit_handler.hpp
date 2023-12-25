#pragma once
#include <CPT/application/probe_design_by_tm/probe_raw_bed.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

struct ProbeHitHandler
{
    template< class DB_BED, class PROBE_BED >
    static void run( DB_BED&& db_bed, PROBE_BED&& probe_bed, int& db_idx)
    {
        const size_t& db_start = std::get<1>( db_bed.data );
        const size_t& db_end   = std::get<2>( db_bed.data );
        const std::string& db_type  = std::get<4>( db_bed.data );

        if( db_type == "db_snp" )
        {
            if( db_end > probe_bed.start_ && db_start < probe_bed.start_+5 )
            {
                probe_bed.l_five++;
            }

            if( db_end > probe_bed.start_+5 && db_start < probe_bed.end_ )
            {
                probe_bed.l_other++;
            }

            if( db_end > probe_bed.end_-5 && db_start < probe_bed.end_ )
            {
                probe_bed.r_five++;
            }

            if( db_end > probe_bed.start_ && db_start < probe_bed.end_-5 )
            {
                probe_bed.r_other++;
            }
        }

        if( db_type == "rmsk" )
        {
            probe_bed.is_rmsk = true;
        }

        if( db_type == "blk_list" )
        {
            probe_bed.is_blklist = true;
        }
    }
};

} // probe_design_by_tm
} // application
} // cpt
