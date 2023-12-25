#pragma once
#include <pokemon/format/annotation_raw_bed.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

template< typename TUPLETYPE = std::tuple <
    std::string, //QNAME
    int, //SAM_FLAG, //FLAG
    std::string, //RNAME
    uint64_t, //POS
    int, //MAPQ
    std::string, //CIGAR
    std::string, //RNEXT
    uint64_t, //PNEXT
    int64_t, //TLEN
    std::string, //SEQ
    std::string, //QUAL
    UserDefineTags< boost::mpl::string<'NH'>, boost::mpl::string<'TL'> >
    //UserDefineContent
>>
struct ProbeRawBed : public AnnotationRawBed< TUPLETYPE >
{
    using Base = AnnotationRawBed< TUPLETYPE >;
    using Base::Base;

    size_t snp_site;

    size_t l_five, l_other;
    size_t r_five, r_other;

    bool is_rmsk, is_blklist;

    ProbeRawBed( std::string chr, size_t start, size_t end, std::string strand )
    : AnnotationRawBed< TUPLETYPE >()
    , snp_site( 0 )
    , l_five( 0 )
    , r_five( 0 )
    , l_other( 0 )
    , r_other( 0 )
    , is_rmsk( false )
    , is_blklist( false )
    {
        basic_setup( chr, start, end, strand.at(0) );
    }

    ProbeRawBed( std::string chr, size_t start, size_t end, std::string strand, size_t snpsite )
    : AnnotationRawBed< TUPLETYPE >()
    , snp_site( snpsite )
    , l_five( 0 )
    , r_five( 0 )
    , l_other( 0 )
    , r_other( 0 )
    , is_rmsk( false )
    , is_blklist( false )
    {
        basic_setup( chr, start, end, strand.at(0) );
    }

    ProbeRawBed( std::string chr, size_t start, size_t end, char strand )
    : AnnotationRawBed< TUPLETYPE >()
    , snp_site( 0 )
    , l_five( 0 )
    , r_five( 0 )
    , l_other( 0 )
    , r_other( 0 )
    , is_rmsk( false )
    , is_blklist( false )
    {
        basic_setup( chr, start, end, strand );
    }

    ProbeRawBed( std::string chr, size_t start, size_t end, char strand, size_t snpsite )
    : AnnotationRawBed< TUPLETYPE >()
    , snp_site( snpsite )
    , l_five( 0 )
    , r_five( 0 )
    , l_other( 0 )
    , r_other( 0 )
    , is_rmsk( false )
    , is_blklist( false )
    {
        basic_setup( chr, start, end, strand );
    }

    void basic_setup( std::string& chr, size_t& start, size_t& end, char& strand )
    {
        Base::start_  = start;
        Base::end_    = end;
        Base::length_ = end - start;

        Base::chr_idx_    = GetChr( chr, strand );
        Base::chromosome_ = Base::GetChr( Base::chr_idx_ );
        Base::strand_     = Base::GetStrand( Base::chr_idx_ );

        Base::reads_count_   = 0;
        Base::tail_length_   = 0;
        Base::tail_mismatch_ = 0;
        Base::multiple_alignment_site_count_ = 0;

        Base::is_filtered_     = 0;
        Base::annotation_info_ = std::vector< std::vector <std::string> >(0);
    }

	uint8_t GetChr( const std::string& chrstr, const char& strand )
	{
        size_t flag = 0;

        switch( strand )
        {
            case '+' : flag =  0; break;
            case '-' : flag = 16; break;
        }

		if( Base::gChrRefMap_.find(chrstr) == Base::gChrRefMap_.end())
		{
			uint8_t tmp = Base::chr_id_gen;
			if( flag == 0 )
			{
                Base::gChrRefMap_.emplace( chrstr, tmp );
                Base::chr_id_gen ++;
				return tmp;
			}
			else
			{
                Base::gChrRefMap_.emplace( chrstr, tmp );
                Base::chr_id_gen ++;
				return tmp + 128 ;
			}
		}
		else
		{
			if( flag == 0 )
				return Base::gChrRefMap_[ chrstr ];
			else
				return( Base::gChrRefMap_[ chrstr ] + 128 );
		}
	}

    friend std::ostream& operator<< ( std::ostream& out, const ProbeRawBed< TUPLETYPE >& probe )
    {
		out << probe.chromosome_ << "\t"
            << probe.start_      << "\t"
            << probe.end_        << "\t"
            << probe.strand_     << "\t"
            << probe.snp_site    << "\n";

        out << "Left:"             << "\t"
            << probe.l_five        << "\t"
            << probe.l_other       << "\t"
            << "Right:"            << "\t"
            << probe.r_five        << "\t"
            << probe.r_other       << "\t"
            << "is_rmsk:"          << "\t"
            << probe.is_rmsk       << "\t"
            << "is_blklist:"       << "\t"
            << probe.is_blklist    << "\n";

        return out;
    }

};

} // probe_design_by_tm
} // application
} // cpt
