#pragma once
#include <CPT/application/probe_design_by_tm/data_pool.hpp>
#include <CPT/application/probe_design_by_tm/aligner_bowtie2.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

class ProbeCheck
{
  public:

    cpt::application::probe_design_by_tm::DataPool& db;
    ThreadSafeCounter& tsc;

    template< class DP >
    ProbeCheck( DP& dp, ThreadSafeCounter& tsc_in )
    : db( dp )
    , tsc( tsc_in )
    {}

    void align_check( std::map< double, ProbeTupleType >& probes_map )
    {
        AlignerBowtie2 aligner( db.aligner_location, db.aligner_index );

        std::string fq_file_name = "temp_fq" + std::to_string( tsc.next() ) + ".fq";
        std::ofstream fq_tmp( fq_file_name );

        size_t idx = 0;
        std::map< size_t, std::map< double, ProbeTupleType >::iterator > map_idx;
        std::map< double, ProbeTupleType >::iterator map_it = probes_map.begin();

        for( auto& probe : probes_map )
        {
            map_idx[ idx ] = map_it;
            std::string& sequence = std::get<4>( probe.second );

            std::mutex fq_mutex;
            write_fq( fq_tmp, idx, sequence, fq_mutex );

            map_it++;
            idx++;
        }

        fq_tmp.close();

        aligner.set_options( "--no-unal --score-min C,0,-1 -k 10" ); // perfect match
        perf_aln_count_parse_sam( map_idx, aligner.file_align( fq_file_name ));

        aligner.set_options( "-N 1 -L 8 -k 10" ); // mismatch match
        mm_aln_count_parse_sam( map_idx, aligner.file_align( fq_file_name ));

        std::remove( fq_file_name.c_str() ); 
    }

    void anno_check_snp6( const size_t& id, const size_t& allele, std::map< double, ProbeTupleType >& probes_map )
    {
        std::string& chromosome = std::get<0>( probes_map.begin()->second );
        char& strand            = std::get<3>( probes_map.begin()->second ).at(0);
        std::string& allele_seq = std::get<5>( db.beds[ id ] )[ allele ];
        size_t& snp_site        = std::get<1>( db.beds[ id ] );
        bool is_genome_seq;

        switch( strand )
        {
            case '+' :
                db.chr_table[ chromosome ].substr( snp_site , 1 ) == allele_seq
                    ? is_genome_seq = true
                    : is_genome_seq = false
                    ;
                break;

            case '-' :
                reverse_complement( db.chr_table[ chromosome ].substr( snp_site , 1 )) == allele_seq
                    ? is_genome_seq = true
                    : is_genome_seq = false
                    ;
                break;
        }

        /**
         *  one probe to 2 sequence
         *           8 snp 5
         *  ------|-----*-----|------
         *         no mismatch
         *  allow1             allow1
        **/

        for( auto& probe : probes_map )
        {
            size_t& probe_start     = std::get< 1>( probe.second );
            size_t& probe_end       = std::get< 2>( probe.second );

            std::string& sequence   = std::get< 4>( probe.second );
            size_t& perf_aln_count  = std::get< 5>( probe.second );
            size_t& mm_aln_count    = std::get< 6>( probe.second );

            bool& is_filter         = std::get< 8>( probe.second );
            size_t& count_5         = std::get< 9>( probe.second );
            size_t& count_other     = std::get<10>( probe.second );
            bool& is_rmsk           = std::get<11>( probe.second );
            bool& is_blklist        = std::get<12>( probe.second );

            std::get< 7>( probe.second ) = is_genome_seq;

            perf_aln_count < 2
                ? is_filter = true
                : is_filter = false
                ;

            if( mm_aln_count > 1 )
            {
                is_filter = false;
            }

            std::vector< ProbeRawBed<> > probe_beds{
                ProbeRawBed<>( chromosome, probe_start, snp_site, strand, snp_site ),
                ProbeRawBed<>( chromosome, snp_site+1, probe_end, strand, snp_site )
            };

            for( auto& probe_bed : probe_beds )
            {
                db.annotator.AnnotateAll( probe_bed );
            }

            count_5 = probe_beds[0].r_five + probe_beds[1].l_five;
            count_other = probe_beds[0].r_other + probe_beds[1].l_other;

            if( probe_beds[0].r_five != 0 || probe_beds[1].l_five != 0 )
            {
                is_filter = false;
            }

            if( probe_beds[0].r_other > 1 || probe_beds[1].l_other > 1 )
            {
                is_filter = false;
            }

            if( probe_beds[0].is_rmsk || probe_beds[1].is_rmsk )
            {
                is_filter = false;
                is_rmsk = true;
            }

            if( probe_beds[0].is_blklist || probe_beds[1].is_blklist )
            {
                is_filter = false;
                is_blklist = true;
            }
        }
    }

    void anno_check_axiom( const size_t& id, const size_t& fwbw, std::map< double, ProbeTupleType >& probes_map )
    {
        std::string& chromosome = std::get<0>( probes_map.begin()->second );
        char& strand            = std::get<3>( probes_map.begin()->second ).at(0);
        size_t& snp_site        = std::get<1>( db.beds[ id ] );

        /**
         *      Allow 1 MM     no MM
         *  ------------------|-----*
         *              or
         *  *-----|------------------
         *   no MM     Allow 1 MM
        **/

        for( auto& probe : probes_map )
        {
            size_t& probe_start     = std::get< 1>( probe.second );
            size_t& probe_end       = std::get< 2>( probe.second );

            std::string& sequence   = std::get< 4>( probe.second );
            size_t& perf_aln_count  = std::get< 5>( probe.second );
            size_t& mm_aln_count    = std::get< 6>( probe.second );

            bool& is_filter         = std::get< 8>( probe.second );
            size_t& count_5         = std::get< 9>( probe.second );
            size_t& count_other     = std::get<10>( probe.second );
            bool& is_rmsk           = std::get<11>( probe.second );
            bool& is_blklist        = std::get<12>( probe.second );

            perf_aln_count < 2
                ? is_filter = true
                : is_filter = false
                ;

            if( mm_aln_count > 1 )
            {
                is_filter = false;
            }

            ProbeRawBed<> probe_bed( chromosome, probe_start, probe_end, strand, snp_site );
            db.annotator.AnnotateAll( probe_bed );

            switch( fwbw )
            {
                case 0 :
                    count_5 = probe_bed.r_five;
                    count_other = probe_bed.r_other;

                    if( probe_bed.r_five != 0 )
                    {
                        is_filter = false;
                    }

                    if( probe_bed.r_other > 1 )
                    {
                        is_filter = false;
                    }

                    break;

                case 1 :
                    count_5 = probe_bed.l_five;
                    count_other = probe_bed.l_other;

                    if( probe_bed.l_five != 0 )
                    {
                        is_filter = false;
                    }

                    if( probe_bed.l_other > 1 )
                    {
                        is_filter = false;
                    }

                    break;
            }

            if( probe_bed.is_rmsk )
            {
                is_filter = false;
                is_rmsk = true;
            }

            if( probe_bed.is_blklist )
            {
                is_filter = false;
                is_blklist = true;
            }
        }
    }

    void perf_aln_count_parse_sam( auto& map_idx, const std::vector<Sam<>>& sam_vec )
    {
        for( auto& sam : sam_vec )
        {
            std::get<5>( map_idx[ std::stoi( std::get<0>( sam.data )) ]->second )++;
        }
    }

    void mm_aln_count_parse_sam( auto& map_idx, const std::vector<Sam<>>& sam_vec )
    {
        for( auto& sam : sam_vec )
        {
            std::get<5>( map_idx[ std::stoi( std::get<0>( sam.data )) ]->second )++;
        }
    }

    /**
     * @brief repeat a string n times.
     * 
     * @param s repeated string
     * @param n repeat times.
     * 
     * @return string s repeated n times's result.
     */
    std::string repeat(const std::string& s, size_t n)
    {
        std::string res("");
        for(int i = 0; i < n; i++)
        {
            res += s;
        }
        return res;
    }
    
    /**
     * @brief write fastq fromated data.
     * 
     * @param stream the stream write to.
     * @param seq the sequence content.
     * @param read_name the read name of fastq.
     * @param fq_mutex the write mutex.
     */   
    void write_fq( auto&& stream, const auto& id, const std::string& seq, std::mutex& fq_mutex)
    {
        std::lock_guard< std::mutex > lock( fq_mutex );
        stream << "@" << id << '\n';
        stream << seq << '\n'; 
        stream << "+" << seq << '\n'; 
        stream << repeat( "#", seq.size() ) << '\n'; 
    }
};

} // probe_design_by_tm
} // application
} // cpt
