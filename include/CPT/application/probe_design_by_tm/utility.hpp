#pragma once
#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <tuple>
#include <map>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

using ProbeBedTupleType = std::tuple< std::string, size_t, size_t, std::string, std::string, std::vector< std::string >>;
                                    //    chr       start    end      strand,     name,        A allele / B allele

using ProbeTupleType = std::tuple< std::string, size_t, size_t, std::string, std::string, size_t, size_t, bool, bool, size_t, size_t, bool, bool >;
// chr / start / end / strand / sequence / perf_aln_count / mm_aln_count / is_genome_seq / is_filter / count_5 / count_other / is_rmsk / is_blklist

bool is_tm( const double& tm, std::pair< double, ProbeTupleType >& last_pair, std::pair< double, ProbeTupleType >& this_pair )
{
    if( this_pair.first == tm )
    {
        return true;
    }

    if( this_pair.first > tm ) 
    {
        if(( this_pair.first - tm ) > ( tm - last_pair.first ))
        {
            this_pair = last_pair;
        }

        return true;
    }

    if( this_pair.first < tm ) 
    {
        if( this_pair.first >= last_pair.first )
        {
            last_pair = this_pair;
        }
    }

    return false;
}

std::string reverse_complement( const std::string& seq )
{
    std::string res{ seq.crbegin(), seq.crend() };

    for( auto& c : res )
    {
        switch( c )
        {
            case 'A': c = 'T'; break;
            case 'T': c = 'A'; break;
            case 'C': c = 'G'; break;
            case 'G': c = 'C'; break;
            default : res = "";break;
            // default : throw std::runtime_error( "Error base " + std::to_string( c ) + " in sequence" );
        }

        if( res.length() == 0 )
        {
            break;
        }
    }

    return res;
}

std::string sequence_is_valid( const std::string& seq )
{   
    for( auto&& c : seq )
    {
        switch( c )
        {
            case 'A' : continue;
            case 'T' : continue;
            case 'C' : continue;
            case 'G' : continue;
            default  : return "";
        }
    }
    return seq;
}

/**
 * @brief chromosome name format.
 * @details tolower and name mapping.
 * 
 * @param db_chrname chromosome name from CCD database.
 * @return return formated chromosome name
 */
std::string chrname_formation( const std::string& db_chrname )
{
    std::string res ( db_chrname );
    res[0] = ::tolower(res[0]); // In database first character is 'C'hrXX but is 'c'hrXX in fasta file.
    if(res == "chr23")  return "chrX";
    if(res == "chr24")  return "chrY";
    if(res == "chr25")  return "chrMT";
    if(res == "chrM")   return "chrMT";
    else                return res;
}

/**
 * @brief a thread safe counter.
 */
struct ThreadSafeCounter
{
    size_t i;
    std::mutex mux;
    ThreadSafeCounter()
    : i(0)
    {}
    /**
     * @brief get the counter next number.
     * @return counter next number.
     */
    size_t next()
    {
        std::lock_guard<std::mutex> lock(mux);
        auto res = i;
        i++;
        return res;
    }
    /**
     * @brief get counter number.
     * @return counter number.
     */
    size_t get()
    {
        return i;
    }
};

} // probe_design_by_tm
} // application
} // cpt
