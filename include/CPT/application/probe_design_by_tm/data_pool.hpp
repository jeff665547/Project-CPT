#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <pokemon/iohandler/ihandler2/ihandler.hpp>
#include <Nucleona/parallel/thread_pool.hpp>
#include <CCD/analyzer/probe_tm.hpp>
#include <CPT/application/probe_design_by_tm/utility.hpp>
#include <CPT/application/probe_design_by_tm/aligner_bowtie2.hpp>
#include <CPT/application/probe_design_by_tm/probe_raw_bed.hpp>
#include <CPT/application/probe_design_by_tm/probe_hit_handler.hpp>
#include <pokemon/annotator/annotation.hpp>
#include <pokemon/annotator/annotation_set.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

using BedFileReaderImpl = FileReader_impl<
      Bed
    , std::tuple< std::string, uint32_t, uint32_t, char, std::string, std::string >
    , SOURCE_TYPE::IFSTREAM_TYPE
>;

using AnnotationTrait = Annotation<
      BedFileReaderImpl
    , AnnoIgnoreStrand::NO_IGNORE
    , AnnoType::INTERSET
    , ProbeHitHandler
>;

using Annotations = AnnotationSet <
      ProbeRawBed<>
    , AnnotationTrait
>;

class DataPool
{
  public:

    boost::property_tree::ptree json_config;

    std::ofstream& output_file;

    std::map< std::string, std::string > chr_table;

    std::string genome_file, bed_file, method_type;

    size_t thread_num, task_num, min_len, max_len, no_snp_len;

    std::vector< ProbeBedTupleType > beds;

    std::vector< std::vector< std::pair< double, ProbeTupleType >>> res_vec;

    double tm, K_mM, divalent, dntp, DNA_nM;

    bool is_probecheck;

    std::string aligner_location, aligner_index; 

    std::vector< std::string > annotation_file;

    Annotations annotator;

    template< class OP >
    DataPool( OP& op )
    : output_file( op.output_file )
    , annotation_file( std::vector< std::string >() )
    , annotator( annotation_file )
    {
        boost::property_tree::read_json( op.json_config, json_config );
        parse_json( json_config );

        method_type == ""
            ? throw std::runtime_error( "the \"method_type\" is need" )
            : true
            ;

        bed_file == ""
            ? throw std::runtime_error( "the \"bed_file\" is need" )
            : get_bed_list( beds, bed_file )
            ;

        set_chr_table( genome_file );
        task_num = (( beds.size() / thread_num ) *2 ) +1;
        
        res_vec = std::vector< std::vector< std::pair< double, ProbeTupleType >>>(
            beds.size(), std::vector< std::pair< double, ProbeTupleType >>(
                2, std::pair< double, ProbeTupleType >()
            )
        );

        if( is_probecheck )
        {
            if( annotation_file.empty() )
            {
                throw std::runtime_error( "the \"annotation_file\" are need for probe checking" );
            }

            annotator.clear_database();
            Annotations anno_temp( annotation_file );
            annotator = std::move( anno_temp );
        }
    }

    void parse_json( boost::property_tree::ptree& ptree )
    {
        // auto& ptree = db.json_config;
        K_mM          = ptree.get_optional< double      >( "K_mM"          ).value_or( 50.0 );
        divalent      = ptree.get_optional< double      >( "divalent"      ).value_or( 0.0  );
        dntp          = ptree.get_optional< double      >( "dntp"          ).value_or( 0.0  );
        DNA_nM        = ptree.get_optional< double      >( "DNA_nM"        ).value_or( 50.0 );
        tm            = ptree.get_optional< double      >( "tm"            ).value_or( 60.0 );
        genome_file   = ptree.get_optional< std::string >( "genome_file"   ).value_or( "/home/john/workdir/genome/hg19.fa" );
        bed_file      = ptree.get_optional< std::string >( "bed_file"      ).value_or( "" );
        method_type   = ptree.get_optional< std::string >( "method_type"   ).value_or( "" );
        thread_num    = ptree.get_optional< size_t      >( "thread_num"    ).value_or( 16 );
        min_len       = ptree.get_optional< size_t      >( "min_len"       ).value_or( 25 );
        max_len       = ptree.get_optional< size_t      >( "max_len"       ).value_or( 50 );
        no_snp_len    = ptree.get_optional< size_t      >( "no_snp_len"    ).value_or( 8  );

        auto probecheck = ptree.get_child_optional( "probe_check" );

        probecheck
            ? is_probecheck = true
            : is_probecheck = false
            ;
        
        if( is_probecheck )
        {
            aligner_location = ptree.get< std::string >( "probe_check.aligner_location" );
            aligner_index    = ptree.get< std::string >( "probe_check.aligner_index" );
            annotation_file.emplace_back( ptree.get< std::string >( "probe_check.annotation_file" ));
        }
    }

    void get_bed_list( std::vector< ProbeBedTupleType >& list, const std::string& path )
    {
        std::string line;
        std::ifstream file( path );
        std::vector< std::string > vec;
        bool snp6 = false;

        if( method_type == "SNP6" )
        {
            snp6 = true;
        }

        while( std::getline( file, line ))
        {
            boost::iter_split( vec, line, boost::algorithm::first_finder( "\t" )); 

            switch( snp6 )
            {
                case false :
                    list.emplace_back( vec[0], std::stoul( vec[1] ), std::stoul( vec[2] ), vec[3], vec[4], std::vector< std::string >() );
                    break;

                case true  :
                    list.emplace_back( vec[0], std::stoul( vec[1] ), std::stoul( vec[2] ), vec[3], vec[4], std::vector< std::string >{ vec[6], vec[7] });
                    break;
            }
            vec.clear();
        }
    }

    void output_probe_res( const bool& is_probe_check )
    {
        output_file << "AlleleBed\tProbeSequence\tSequenceLength\tProbeTM\tIsGenomeSeq";

        if( is_probe_check )
        {
            output_file << "\tIsProbeFilter\tPerfAlnCount\tMMAlnCount\tSNP5\tSNPOther\tIsRMSK\tIsBlkList";
        }

        output_file << "\n";

        for( size_t id = 0; id < beds.size(); ++id )
        {
            output_file << beds[ id ];

            for( size_t allele = 0; allele < res_vec[ id ].size(); ++allele )
            {
                double& probe_tm        = res_vec[ id ][ allele ].first;

                std::string& chromosome = std::get< 0>( res_vec[ id ][ allele ].second );
                size_t& probe_start     = std::get< 1>( res_vec[ id ][ allele ].second );
                size_t& probe_end       = std::get< 2>( res_vec[ id ][ allele ].second );
                std::string& strand     = std::get< 3>( res_vec[ id ][ allele ].second );
                std::string& sequence   = std::get< 4>( res_vec[ id ][ allele ].second );
                size_t& perf_aln_count  = std::get< 5>( res_vec[ id ][ allele ].second );
                size_t& mm_aln_count    = std::get< 6>( res_vec[ id ][ allele ].second );
                bool& is_genome_seq     = std::get< 7>( res_vec[ id ][ allele ].second );
                bool& is_filter         = std::get< 8>( res_vec[ id ][ allele ].second );
                size_t& count_5         = std::get< 9>( res_vec[ id ][ allele ].second );
                size_t& count_other     = std::get<10>( res_vec[ id ][ allele ].second );
                bool& is_rmsk           = std::get<11>( res_vec[ id ][ allele ].second );
                bool& is_blklist        = std::get<12>( res_vec[ id ][ allele ].second );

                output_file << "\t" << sequence << "\t" << sequence.length() << "\t" << probe_tm << "\t" << bool_out( is_genome_seq );

                if( is_probe_check )
                {
                    output_file << "\t" << bool_out( is_filter )<< "\t" << perf_aln_count << "\t" << mm_aln_count << "\t" << count_5 << "\t" << count_other << "\t" << bool_out( is_rmsk )<< "\t" << bool_out( is_blklist );
                }
            }

            output_file << "\n";
        }
    }

    std::string bool_out( const bool& bool_ )
    {
        std::string res;
        switch( bool_ )
        {
            case true  : res = "ture";  break;
            case false : res = "false"; break;
        }
        return res;
    }

    /**
     * @brief read chromosome table from file, and standardize (transform and name formated .
     * 
     * @param path the chromosome file, fasta format.
     * @param thread_num use number of thread to transform.
     */
    void set_chr_table( const std::string& path, const size_t& thread_num = 16 )
    {
        IHandler < 
            ParallelTypes::NORMAL, 
            Fasta, 
            std::tuple< std::string, std::string >, 
            IoHandlerIfstream > fa_reader( path );
        auto thread_pool(nucleona::parallel::make_thread_pool(thread_num));
        while( true )
        {
            Fasta<> fa_obj;
            fa_obj = fa_reader.get_next_entry();

            if( fa_obj.eof_flag )
                break;

            std::string chrname = chrname_formation( std::get<0>( fa_obj.data ));
            std::string& chrseq = std::get<1>( fa_obj.data );
            chr_table[ chrname ] = std::move( chrseq );

            std::function< void( void ) > task = [
                this, 
                chrname{ std::move( chrname )}, 
                &thread_pool 
            ]()
            {
                parallel_transform(
                        chr_table[ chrname ].begin(), 
                        chr_table[ chrname ].length(),
                        chr_table[ chrname ].begin(),
                        thread_pool
                );
            };

            thread_pool.job_post( task );
        }
        thread_pool.flush();
    }

    /**
     * @brief parallel character to upper transform.
     * @details block wise transform.
     * @param src_begin input sequence begin.
     * @param length sequence length.
     * @param tar_begin output sequence begin.
     * @param parallel_pool Multi-thread pool.
     */
    void parallel_transform( auto src_begin, size_t&& length, auto tar_begin, auto&& parallel_pool )
    {
        size_t num ( parallel_pool.get_thread_num());
        size_t block_size ( length / num );
        size_t remain ( length % num );
        for(size_t i (0); i < num - 1; i++)
        {
            parallel_pool.job_post([i, src_begin, tar_begin, block_size]()
            {
                std::transform(
                        src_begin + ( i * block_size), 
                        src_begin + (( i + 1) * block_size), 
                        tar_begin + ( i * block_size), 
                        ::toupper
                );
            });
        }
        std::transform( 
                src_begin + ((num - 1) * block_size), 
                src_begin + length, 
                src_begin + ((num - 1) * block_size),
                ::toupper
        );
    }
};

} // probe_design_by_tm
} // application
} // cpt
