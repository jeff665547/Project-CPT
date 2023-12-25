#pragma once
#include <cstdint>
#include <string>
#include <regex>
#include <CPT/forward.hpp>
#include <CPT/logger.hpp>
#include <CPT/exception.hpp>
#include <CPT/format/json.hpp>
#include <CPT/format/cdffile.hpp>
#include <CPT/format/axiom/annot_csv.hpp>
#include <CPT/format/axiom/probe_tab.hpp>
#include <CPT/format/cad.hpp>
#include <CCD/utility/sequence_operation.hpp>
// #include <boost/range/algorithm/copy.hpp>
// #include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <unordered_set>
#include <CPT/algorithm/lazy_foreach.hpp>

namespace se = std::experimental;
namespace cf = cpt::format;
namespace ce = cpt::exception;
namespace ba = boost::adaptors;
namespace br = boost::range;

namespace cpt {
namespace format {
namespace converter {

// template<class VS>
// std::string append_vs( const VS& vs )
// {
//     std::string res = vs[0];
//     for( std::size_t i (1); i < vs.size(); i ++ )
//     {
//         res += ("." + vs[i]);
//     }
//     return res;
// }

uint16_t to_channel_code(const char base)
{
    if (base == 'A' or base == 'T')
        return 1;
    else if (base == 'C' or base == 'G')
        return 0;
    else
        throw std::invalid_argument("invalid base to channel conversion");
}

class AxiomProbe
{
  public:
    static const char sep = '/';
    std::map<uint8_t, std::pair<uint8_t, char>> des;

    uint32_t shape;
    std::tuple<uint32_t, uint32_t, uint16_t, uint16_t> region;
    uint32_t start;
    uint16_t length;
    char strand;
    std::string content;

    AxiomProbe(void)
      : shape(0)
      , region({ 0, 0, 0, 0 })
      , start(0)
      , length(0)
      , strand('.')
      , content("---")
    {}

    auto get_sequence(void)
    {
        auto json = cpt::format::make_json();
        json.add("start"  , start  );
        json.add("length" , length );
        json.add("strand" , strand );
        json.add("content", content);
        return json.root;
    }
    auto get_channel_des(void)
    {
        std::string alleles, channels, bases;
        auto itr = des.begin();
        if (!des.empty())
        {
            alleles  += { (char)(itr->first + 'A') };
            channels += { (char)(itr->second.first + '0') };
            bases    += { itr->second.second };
            itr++;
        }
        while (itr != des.end())
        {
            alleles  += { sep, (char)(itr->first + 'A') };
            channels += { sep, (char)(itr->second.first + '0') };
            bases    += { sep, itr->second.second };
            itr++;
        }
        auto json = cpt::format::make_json();
        json.add("allele" , alleles );
        json.add("channel", channels);
        json.add("base"   , bases   );
        return json.root;
    }
};

class AxiomSequence
{
    using This = AxiomSequence;

  private:
    uint32_t start_;
    uint16_t length_;
    char strand_;
    char ext_[2];
    std::string seq_[2];

  private:
    template <class SEQ, class... POS>
    static auto reverse(SEQ&& seq, POS&&... pos)
    {
        std::remove_reference_t<SEQ> res;
        boost::copy(
            seq.substr(pos...)
          | cpt::algorithm::lazy_foreach([](auto&& c){ return ::toupper(c); })
          | ba::reversed
          , std::back_inserter(res)
        );
        return res;
    }
    
    template <class SEQ, class... POS>
    static auto complement(SEQ&& seq, POS&&... pos)
    {
        std::remove_reference_t<SEQ> res;
        boost::copy(
            seq.substr(pos...)
          | cpt::algorithm::lazy_foreach([](auto&& c){ return ::toupper(c); })
          | cpt::algorithm::lazy_foreach(::complement_alow_N)
          , std::back_inserter(res)
        );
        return res;
    }

  public:
    auto content(const uint32_t allele_code) const
      { return seq_[allele_code]; }
    auto base(const uint32_t allele_code) const
      { return ext_[allele_code]; }
    auto length(void) const
      { return length_; }
    auto strand(void) const
      { return strand_; }
    auto start(void) const
      { return start_; }

    AxiomSequence(
        cf::axiom::AnnotCsv::Entry* a
      , cf::axiom::ProbeTab::Entry* p
    )
      : start_(0)
      , length_(0)
      , strand_('.')
      , ext_{'N', 'N'}
      , seq_{"---" , "---" }
    {
        if (p == nullptr or a == nullptr or a->flankseq.empty())
        {
            return;
        }
        else if (p->probeseq.front() == '[')
        {
            auto pos = p->probeseq.find(']');
            auto loc = a->flankseq.find('[');
            if (p->allele_a == '-')
            {
                ext_[0] = a->flankseq.at(loc - 1);
                ext_[1] = a->allele_b;
                seq_[1] = seq_[0] = This::reverse(p->probeseq, pos + 1);
                length_ = seq_[0].length();
                start_  = a->start - 1;

                // TODO indel with covSNP design
            }
            else
            {
                if (to_channel_code(p->allele_a) == to_channel_code(p->allele_b))
                {
                    ext_[0] = ext_[1] = a->flankseq.at(loc - 1);
                    seq_[1] = seq_[0] = This::reverse(p->probeseq, pos + 1);
                    seq_[0].push_back(p->allele_a);
                    seq_[1].push_back(p->allele_b);
                    length_ = seq_[0].length();
                    start_  = a->start;
                }
                else
                {
                    ext_[0] = p->allele_a;
                    ext_[1] = p->allele_b;
                    seq_[1] = seq_[0] = This::reverse(p->probeseq, pos + 1);
                    length_ = seq_[0].length();
                    start_  = a->start - 1;
                }
            }
            strand_ = '+';
        }
        else if (p->probeseq.back() == ']')
        {
            auto pos = p->probeseq.find('[');
            auto loc = a->flankseq.find(']');
            if (p->allele_a == '-')
            {
                ext_[0] = ::complement_alow_N(a->flankseq.at(loc + 1));
                ext_[1] = ::complement_alow_N(a->allele_b);
                seq_[1] = seq_[0] = This::complement(p->probeseq, 0, pos);
                length_ = seq_[0].length();
                start_  = a->end - length_ - 1;
                
                // TODO indel with covSNP design
            }
            else
            {
                if (to_channel_code(p->allele_a) == to_channel_code(p->allele_b))
                {
                    ext_[0] = ext_[1] = ::complement_alow_N(a->flankseq.at(loc + 1));
                    seq_[1] = seq_[0] = This::complement(p->probeseq, 0, pos);
                    seq_[0].push_back(::complement_alow_N(p->allele_a));
                    seq_[1].push_back(::complement_alow_N(p->allele_b));
                    length_ = seq_[0].length();
                    start_  = a->end - length_;
                }
                else
                {
                    ext_[0] = ::complement_alow_N(p->allele_a);
                    ext_[1] = ::complement_alow_N(p->allele_b);
                    seq_[1] = seq_[0] = This::complement(p->probeseq, 0, pos);
                    length_ = seq_[0].length();
                    start_  = a->end - length_ - 1;
                }
                strand_ = '-';
            }
        }
        else throw std::runtime_error("Unknown annot or probe_tab status");
    }
};

class AxiomCdf2CadConverter
{
  public:
    // using ProbesetType = cf::cad::ProbesetType<cf::cad::Probe>;
    // using ProbesetT = cf::cad::Probeset<
    //       cf::cad::Probe
    //     , ProbesetType
    // >;
    // std::unique_ptr<cfc::ChannelDes> extract_channel_des(
    //       const cf::CDFFile&  cdf
    //     , const cf::CDFUnit&  unit
    //     , const cf::CDFBlock& block
    //     , const cf::CDFCell&  cell
    //     , ProbesetType& probeset_type
    //     , const typename cfa::ProbTab::Entry* ptentry
    // )
    // {
    //     return probeset_type.extract_channel_des_axiomcdf(cdf, unit, block, cell, ptentry);
    // }
    // std::unique_ptr<cf::cad::ProbeSeq> extract_probe_seq(
    //       const cf::CDFFile&  cdf
    //     , const cf::CDFUnit&  unit
    //     , const cf::CDFBlock& block
    //     , const cf::CDFCell&  cell
    // )
    // {
    //     return std::unique_ptr<cf::cad::ProbeSeq>(new cf::cad::NoProbeSeq()); 
    // }
    // std::unique_ptr<ProbesetType> extract_probeset_type( const cf::CDFUnit& unit )
    // {
    //     // TODO parameter not set
    //     return std::unique_ptr<ProbesetType>(new cfc::GenotypingLigationBased<cfc::Probe>());
    // }
    // auto get_probeset_list(cpt::format::CDFFile& cdffile, cfa::ProbTab& pt)
    // {
    //     using namespace cpt::format;

    //     std::vector<ProbesetT> probeset_list;
    //     for( auto&& unit : cdffile.units )
    //     {
    //         std::vector<cad::Probe> probe_list;
    //         auto probeset_type ( 
    //             extract_probeset_type(
    //                 unit
    //             )
    //         );
    //         auto* ptentry = pt.get_entry(unit.name);
    //         for ( auto&& block : unit.blocks )
    //         {
    //             for ( auto&& cell : block.cells )
    //             {
    //                 probe_list.emplace_back(
    //                       cell.id
    //                     , 0
    //                     , std::make_tuple    ( cell.id, cell.id, 0, 0 )   // id
    //                     , extract_channel_des(                      // channel_des
    //                           cdffile
    //                         , unit 
    //                         , block
    //                         , cell
    //                         , *probeset_type
    //                         , ptentry
    //                     )
    //                     , extract_probe_seq(                        // probe_seq
    //                           cdffile
    //                         , unit
    //                         , block
    //                         , cell
    //                     )
    //                 );
    //             }
    //         }
    //         probeset_list.emplace_back(
    //               std::move(probeset_type)  // type       ( probeset type include channel info  )
    //             , unit.name                 // name
    //             , std::string("")           // chrom      ( target chromosome                   )
    //             , 0                         // start      ( target start                        )
    //             , 0                         // end        ( target end                          )
    //             , 'u'                       // strand     ( target strand                       )
    //             , std::move(probe_list)     // probe_list 
    //         ); // TODO 
    //     }
    //     return probeset_list;
    // }
    // public:
    // Cad<ProbesetT> to_cad ( cpt::format::CDFFile& cdffile, cfa::ProbTab& pt )
    // {
    //     return std::move(Cad<ProbesetT> (
    //           113                                               // magic
    //         , "000"                                             // version
    //         , "CDF." + append_vs(cdffile.probe_array_types)     // probe_array_type.name
    //         , "0"                                               // probe_array_type.version
    //         , cdffile.num_cols * cdffile.num_rows               // num_features
    //         , cdffile.num_cols                                  // num_cols
    //         , cdffile.num_rows                                  // num_rows
    //         , 2                                                 // num_channels ( hardcode )
    //         , "undefined"                                       // genome_assembly
    //         , "3-5"                                             // probe_direction
    //         , get_probeset_list(cdffile, pt)                        // probeset_list
    //     ));
    //     // TODO
    // }
    // template<class IS, class SP >
    // auto initial_cad ( IS&& is, SP&& pt)
    // {
    //     cpt::format::CDFFile cdffile;
    //     cdffile.open(is);
    //     auto cad ( to_cad( cdffile, pt ) );
    //     return cad;
    // }
    // auto get_snp_form ( char a, char b, int seq_len )
    // {
    //     if      ( a == 'A' && b == 'T' ) return cfc::SNPForm::coverSNP;
    //     else if ( a == 'C' && b == 'G' ) return cfc::SNPForm::coverSNP;
    //     else if ( a == 'A' && b == 'C' ) return cfc::SNPForm::extSNP;
    //     else if ( a == 'A' && b == 'G' ) return cfc::SNPForm::extSNP;
    //     else if ( a == 'T' && b == 'C' ) return cfc::SNPForm::extSNP;
    //     else if ( a == 'T' && b == 'G' ) return cfc::SNPForm::extSNP;
    //     else 
    //     {
    //         if      ( seq_len == 31 ) return cfc::SNPForm::extSNP;
    //         else if ( seq_len == 30 ) return cfc::SNPForm::coverSNP;
    //         else throw cpt::exception::UnknownSNPForm ("");
    //     }
    // }
    // auto get_snp_form ( const typename cfa::ProbTab ::Entry& ptentry)
    // {
    //     return get_snp_form(
    //           ptentry.allele_a
    //         , ptentry.allele_b
    //         , ptentry.probeseq.length() - 4
    //     ) ;
    // }
    // auto get_ref_info( 
    //       const typename cfa::ProbTab ::Entry& ptentry
    //     , const typename cfa::AnnotCsv::Entry& acentry
    // )
    // {
    //     // 1. check fw rw form
    //     // 2. check coverSNP of noncover
    //     // 3. fw -> SNP - offset, rw -> SNP 
    //     // auto&& seq_len = ptentry.probeseq.length() - 4;
    //     if ( ptentry.probeseq.at(0) == '[' ) // rw
    //     {
    //         switch ( get_snp_form(
    //               ptentry.allele_a
    //             , ptentry.allele_b
    //             , ptentry.probeseq.length() - 4) 
    //         )
    //         {
    //             case cfc::SNPForm::extSNP:
    //                 return cfc::RefLocation ( 
    //                       acentry.start + 1
    //                     , ( ptentry.probeseq.length() - 5 )
    //                     , '+'
    //                 );
    //             break;
    //             case cfc::SNPForm::coverSNP:
    //                 return cfc::RefLocation (
    //                       acentry.start
    //                     , ( ptentry.probeseq.length() - 4 )
    //                     , '+'
    //                 );
    //             break;
    //         }
    //     }
    //     else if ( ptentry.probeseq.back() == ']' ) // fw
    //     {
    //         switch ( get_snp_form(
    //               ptentry.allele_a
    //             , ptentry.allele_b
    //             , ptentry.probeseq.length() - 4
    //         ))
    //         {
    //             case cfc::SNPForm::extSNP:
    //             {
    //                 auto&& seq_len = ptentry.probeseq.length() - 5;
    //                 return cfc::RefLocation(
    //                       acentry.start 
    //                         - 1 // probe end
    //                         - seq_len
    //                         + 1 // recover pos
    //                     , seq_len
    //                     , '-'
    //                 );
    //             }
    //             break;
    //             case cfc::SNPForm::coverSNP:
    //             {
    //                 auto&& seq_len = ptentry.probeseq.length() - 4;
    //                 return cfc::RefLocation(
    //                       acentry.start 
    //                         - 1 // probe end
    //                         - seq_len
    //                         + 1 // recover pos
    //                     , seq_len
    //                     , '-'
    //                 );
    //             }
    //             break;
    //         }
    //     }
    //     else
    //     {
    //         throw std::logic_error("bad probe seq form");
    //     }
    //     return cfc::RefLocation();
    // }
    // void seq_upper_complement(std::string& seq)
    // {
    //     for(auto& c : seq )
    //     {
    //         c = ::complement((char)std::toupper(c));
    //     }
    // }
    // auto probe_seq_standarize ( 
    //       const typename cfa::ProbTab::Entry& ptentry
    //     , const typename cfa::AnnotCsv::Entry& atentry
    //     , const char allele
    // )
    // {
    //     // TODO
    //     std::string seq;
    //     if( ptentry.probeseq.at(0) == '[' ) //SNP at 5P 
    //     {
    //         seq = std::to_string(
    //             allele == 'A' ? ptentry.allele_a : ptentry.allele_b 
    //         ) + ptentry.probeseq.substr(5);
    //         char ec = seq[0];
    //         char cc = atentry.flankseq[atentry.flankseq.find("[") - 1];
    //         switch (get_snp_form(ptentry))
    //         {
    //             case cfc::SNPForm::extSNP:
    //                 // seq[0] is pbase
    //                 seq = seq.substr(1);
    //                 return std::make_pair(seq, ec);
    //             break;
    //             case cfc::SNPForm::coverSNP:
    //                 // seq[-1] is pbase
    //                 return std::make_pair(seq, cc);
    //             break;
    //         }
    //     }
    //     else if ( ptentry.probeseq.back() == ']' ) // SNP at 3P
    //     {
    //         seq = std::string( 
    //               ptentry.probeseq.begin()
    //             , ptentry.probeseq.end() - 5 
    //         ) + std::to_string( 
    //             allele == 'A' ? ptentry.allele_a : ptentry.allele_b 
    //         );
    //         char cc = ::complement(atentry.flankseq[atentry.flankseq.find("]") + 1]);
    //         char ec = ::complement(seq.back());
    //         switch (get_snp_form(ptentry))
    //         {
    //             case cfc::SNPForm::extSNP:
    //                 // complement ( seq.back() ) is pbase
    //                 seq = seq.substr(0, seq.length()-1);
    //                 seq_upper_complement(seq);
    //                 return std::make_pair(seq, ec);

    //             break;
    //             case cfc::SNPForm::coverSNP:
    //                 // complement ( seq.back() + 1 ) is pbase
    //                 seq_upper_complement(seq);
    //                 return std::make_pair(seq, cc);
    //                 

    //             break;
    //         }
    //     }
    //     else
    //     {
    //         throw std::logic_error ( "unknow SNP posistion" );
    //     }
    //     return std::make_pair(std::string(), '-');
    //     // return std::string(ps);
    // }
    // auto probe_seq_standarize ( 
    //       const typename cfa::ProbTab::Entry& ptentry
    //     , const char allele
    // )
    // {
    //     // TODO
    //     // if is extSNP -> 
    //     //
    //     std::string seq;
    //     if( ptentry.probeseq.at(0) == '[' ) 
    //     {
    //         seq = std::to_string(
    //             allele == 'A' ? ptentry.allele_a : ptentry.allele_b 
    //         ) + ptentry.probeseq.substr(5);
    //         switch (get_snp_form(ptentry))
    //         {
    //             case cfc::SNPForm::coverSNP:
    //                 return seq;
    //             case cfc::SNPForm::extSNP:
    //                 return std::string("");
    //             break;
    //         }
    //     }
    //     else if ( ptentry.probeseq.back() == ']' ) // SNP at 3P
    //     {
    //         seq = std::string( 
    //               ptentry.probeseq.begin()
    //             , ptentry.probeseq.end() - 5 
    //         ) + std::to_string( 
    //             allele == 'A' ? ptentry.allele_a : ptentry.allele_b 
    //         );
    //         switch (get_snp_form(ptentry))
    //         {
    //             case cfc::SNPForm::coverSNP:
    //                 seq_upper_complement(seq);
    //                 return seq;
    //             case cfc::SNPForm::extSNP:
    //                 return std::string("");
    //             break;
    //         }
    //     }
    //     else
    //     {
    //         throw std::logic_error ( "unknow SNP posistion" );
    //     }
    //     return std::string();
    // }
    // void update_cad( 
    //       Cad<ProbesetT>& cad
    //     , cfa::ProbTab& probe_tab
    //     , cfa::AnnotCsv& annot_csv )
    // {
    //     for ( auto& pset : cad.probeset_list )
    //     {
    //         auto* p_annot = annot_csv.get_entry(pset.name);
    //         auto* p_probeset = probe_tab.get_entry(pset.name);
    //         #define ANNOT_VALI p_annot != nullptr && annot_csv.is_solid(*p_annot)
    //         try
    //         {
    //             if ( 
    //                    ANNOT_VALID
    //                 && p_probeset != nullptr
    //             )
    //             {
    //                 /* update SNP info */
    //                 pset.start  = p_annot->start    ;
    //                 pset.end    = p_annot->end      ;
    //                 pset.strand = p_annot->strand   ;
    //                 pset.chrom  = p_annot->chrname  ;
    //                 /* update probe seq, use RefWithSeqContent */
    //                 for ( auto& p : pset.probe_list )
    //                 {
    //                     auto& channel_des = p.channel_des<cfc::AlleleBaseMapChannelDes>();
    //                     auto&& std_prb_seq = probe_seq_standarize(*p_probeset, *p_annot, channel_des.allele);
    //                     try
    //                     {
    //                         p.set_probe_seq(std::unique_ptr<cfc::ProbeSeq>(
    //                             new cfc::RefWithSeqContent(
    //                                   get_ref_info( *p_probeset, *p_annot )
    //                                 , std_prb_seq.first
    //                             )
    //                         ));
    //                     }
    //                     catch(const ce::UnknownSNPForm& e)
    //                     {
    //                         p.set_probe_seq(std::unique_ptr<cfc::ProbeSeq>(
    //                             new cfc::NoProbeSeq()
    //                         ));
    //                         cpt::warn << pset.name << "reference info extract fail, give up" << std::endl;
    //                     }
    //                     pset.probeset_type->update_channel_des_axiom(
    //                           p.channel_des_ptr()
    //                         , std_prb_seq.second
    //                     );

    //                 }
    //             }
    //             else if ( 
    //                    !(ANNOT_VALID)
    //                 && p_probeset != nullptr
    //             )
    //             {
    //                 /* update probe seq, use NoRefInfo */
    //                 for ( auto& p : pset.probe_list )
    //                 {
    //                     // seq only
    //                     auto& channel_des = p.channel_des<cfc::AlleleBaseMapChannelDes>();
    //                     p.set_probe_seq( 
    //                         std::unique_ptr<cfc::ProbeSeq>(
    //                             new cfc::NoRefInfo( 
    //                                 probe_seq_standarize(
    //                                       *p_probeset
    //                                     , channel_des.allele
    //                                 )
    //                             )
    //                         )
    //                     );

    //                 }
    //             }
    //             // else if (
    //             //        ANNOT_VALID
    //             //     && p_probeset == nullptr
    //             // )
    //             // {
    //             //     for ( auto& p : pset.probe_list )
    //             //     {
    //             //         p.set_probe_seq(std::unique_ptr<cfc::ProbeSeq>(
    //             //             new cfc::NoProbeSeq()
    //             //         ));
    //             //     }
    //             // }
    //             else
    //             {
    //                 for ( auto& p : pset.probe_list )
    //                 {
    //                     p.set_probe_seq(std::unique_ptr<cfc::ProbeSeq>(
    //                         new cfc::NoProbeSeq()
    //                     ));
    //                 }
    //             }
    //         }
    //         catch ( const ErrorBaseInSequence& e )
    //         {
    //             if ( e.get_error_base() == '-' ){}
    //             else
    //             {
    //                 abort();
    //             }
    //         }
    //         #undef ANNOT_VALID
    //         // TODO Collect channel des
    //     }
    // }

    auto operator()(
        const std::string& path_to_cdf
      , const std::string& path_to_cad
      , const std::string& path_to_probe_tab
      , const std::string& path_to_annot_csv
    )
    {
        // load probe_tab
        cf::axiom::ProbeTab probe_tab;
        if (bfs::exists(path_to_probe_tab))
        {
            cpt::msg << path_to_probe_tab << " - found\n";
            std::ifstream is(path_to_probe_tab);
            probe_tab.read(is);
            is.clear();
            is.close();
        }

        // load annot_csv
        cf::axiom::AnnotCsv annot_csv;
        if (bfs::exists(path_to_annot_csv))
        {
            cpt::msg << path_to_annot_csv << " - found\n";
            std::ifstream is(path_to_annot_csv);
            annot_csv.read(is);
            is.clear();
            is.close();
        }

        // load cdf
        cf::CDFFile cdf;
        if (bfs::exists(path_to_cdf))
        {
            cpt::msg << path_to_cdf << " - found\n";
            std::ifstream is(path_to_cdf);
            cdf.open(is);
        }
        else throw std::invalid_argument(path_to_cdf + " - not found!");

        cpt::msg << "start CDF to CAD conversion\n";

        // estimate the amount of probes
        std::vector<uint16_t> num_probes(cdf.num_units, 0);
        uint32_t total_probes = 0u;
        {
            for (auto i = 0u; i != cdf.num_units; ++i)
            {
                std::unordered_set<uint32_t> ids;
                auto& unit = cdf.units[i];
                for (auto& block: unit.blocks)
                    for (auto& cell: block.cells)
                        ids.insert(cell.id);
                num_probes[i] = ids.size();
                total_probes += ids.size();
            }
        }

        // create a cad file
        cf::Cad cad;
        cad.create_cad(path_to_cad, cdf.num_units, total_probes);
        cad.magic(113);
        cad.version("000");
        { 
            bfs::path name(path_to_cdf);
            cad.array_type_name(name.filename().string());
            cad.array_type_version("0");
        }
        cad.num_channels(2);
        cad.num_rows(cdf.num_rows);
        cad.num_cols(cdf.num_cols);
        cad.max_seq_length(40);
        cad.max_chn_items(4);
        cad.genome_assembly("hg19");
        cad.probe_direction("3-5");
        auto&& probeset_list = cad.get_probeset_list();
        assert( probeset_list.size() == 0 );

        auto debug_tp = total_probes;
        // fill in probesets
        for (auto i = 0u; i != cdf.num_units; ++i)
        {
            assert( debug_tp - num_probes[i] >= 0 );
            auto&& unit = cdf.units[i];
            auto&& a = annot_csv.get_entry(unit.name);
            auto&& p = probe_tab.get_entry(unit.name);
            auto&& probeset = probeset_list.create_probeset(num_probes[i]);

            // debug_tp -= num_probes[i];
            // assert( debug_tp >= 0 );
        
            probeset.name(unit.name);
            probeset.type("Genotyping");
            probeset.subtype("LigationBased");
            if (a != nullptr)
            {
                probeset.chrom (a->chrname);
                probeset.start (a->start);
                probeset.end   (a->end);
                probeset.strand(a->strand);
                probeset.desc  ({
                    a->allele_a
                  , a->allele_b
                });
            }
            else
            {
                // probeset.chrom ("");
                // probeset.start (0);
                // probeset.end   (0);
                // probeset.strand('.');
                probeset.desc  ({
                    'N'
                  , 'N'
                });
            }
        
            AxiomSequence seq(a, p);
            std::map<uint32_t, AxiomProbe> map;
            for (auto&& block: unit.blocks)
            {
                auto&& allele_code  = block.allele_code;
                auto&& channel_code = block.channel;
        
                for (auto&& cell: block.cells)
                {
                    auto&& ret = map.try_emplace(cell.id, AxiomProbe());
                    auto&& probe = ret.first->second;
                    if (ret.second)
                    {
                        probe.shape   = 0;      
                        probe.region  = { cell.x, cell.y, 1, 1 };
                        probe.start   = seq.start();
                        probe.length  = seq.length();
                        probe.strand  = seq.strand();
                        probe.content = seq.content(allele_code);
                    }
                    auto&& des = probe.des[allele_code];
                    des.first  = channel_code;
                    des.second = seq.base(allele_code);
                }
            }
            auto&& probe_list = probeset.get_probe_list();
            for (auto&& item: map)
            {
                auto&& probe = probe_list.create_probe();
                auto&& p = item.second;
                auto&& cell_id = item.first;
                probe.probe_name(cell_id);
                probe.shape_name(p.shape);
                probe.region_des(p.region);
                probe.channel_des(p.get_channel_des());
                probe.sequence(p.get_sequence());
            }
            
            if ((i + 1) % 1000 == 0)
                std::cerr << "[" << i + 1 << "]";
        }
        std::cerr << '\n';

        // cpt::msg << "start convert CDF to CAD" << std::endl;
        // cfa::ProbTab pt;
        // cpt::msg << "read probe_tab" << std::endl;
        // pt.read(probe_tab_s);
        // cpt::msg << "read probe_tab finish" << std::endl;
        // auto cad = initial_cad(
        //       std::forward<IS>(is)
        //       , pt
        // );
        // cpt::msg << "CAD main data ready" << std::endl;
        // if( annot_csv_s.is_open() )
        // {
        //     cpt::msg << "find the probe_tab file and annot_csv file" << std::endl;
        //     cfa::AnnotCsv annot_csv;
        //     cpt::msg << "read annot_csv" << std::endl;
        //     annot_csv.read(annot_csv_s);
        //     cpt::msg << "read annot_csv finish" << std::endl;
        //     cpt::msg << "start update CAD" << std::endl;
        //     update_cad(cad, pt, annot_csv);
        //     cpt::msg << "update CAD finish" << std::endl;
        // }
        // cpt::msg << "CAD archive" << std::endl;
        // {
        //     boost::archive::text_oarchive oa ( os );
        //     oa << cad;
        // }
        // cpt::msg << "CAD archive done" << std::endl;
        // cad.print(std::cout, 0);
    }
};

}}}
