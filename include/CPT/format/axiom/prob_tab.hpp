#pragma once
#include <CPT/utility/ltuple.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <experimental/string_view>
#include <boost/range/adaptor/indexed.hpp>
#include <map>
namespace cpt {
namespace format {
namespace axiom { 

template<
      class PSI
    , class PROBE_SEQ
    , class A
    , class B
>
struct ProbeTabEntry
{
    PSI         probeset_id;
    PROBE_SEQ   probeseq;
    A           allele_a;
    B           allele_b;
    ProbeTabEntry(
          PSI       &&   probeset_id
        , PROBE_SEQ &&   probeseq
        , A         &&   allele_a
        , B         &&   allele_b
    )
    : probeset_id  (std::move( probeset_id  )) 
    , probeseq     (std::move( probeseq     ))  
    , allele_a     (std::move( allele_a     ))  
    , allele_b     (std::move( allele_b     ))  
    {}
};

using ProbeTabParseTrait = cpt::format::TraitPrototype<
    ProbeTabEntry
    , cpt::format::PMT<0 , std::string> // probe set id
    , cpt::format::PMT<1 , std::string> // probe seq
    , cpt::format::PMT<2 , char> // Allele A
    , cpt::format::PMT<3 , char> // Allele B
>;

using ProbeTabParser = cpt::format::TupleParser<ProbeTabParseTrait>;

struct ProbeTab : public std::vector< typename ProbeTabParseTrait::Entry >
{
    using Entry = typename ProbeTabParseTrait::Entry;
    std::map<std::experimental::string_view, std::size_t> probeset_id_idx;
    template<class IS>
    void read( IS&& is )
    {
        namespace ba = boost::adaptors;
        ProbeTabParser parser;
        std::string line;
        bool header = false;
        while( std::getline(is, line))
        {
            if ( header == false ) { header = true; continue; };
            this->emplace_back(parser(line));
        }
        for( auto&& ientry : *this | ba::indexed(0))
        {
            probeset_id_idx.emplace(
                  ientry.value().probeset_id
                , ientry.index()
            );
        }
    }
    Entry* get_entry( const std::experimental::string_view& probeset_id )
    {
        auto itr ( probeset_id_idx.find ( probeset_id ) );
        if ( itr == probeset_id_idx.end() ) return nullptr;
        else return &(this->at(itr->second));
    }
};

}}}
