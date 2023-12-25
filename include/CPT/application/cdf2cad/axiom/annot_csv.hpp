#pragma once
#include <CPT/utility/ltuple.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <experimental/string_view>
#include <map>
#include <boost/range/adaptor/indexed.hpp>
// /CPT/src/application/cdf2cad/axiom
namespace cpt {
namespace application {
namespace cdf2cad {
namespace axiom { 
// class AnnotCsvIdx;

template<
      class PSI
    , class CHR
    , class START
    , class END
    , class STAND
    , class FLANK
    , class A
    , class B
>
struct AnnotCsvEntry
{
    PSI     probeset_id;
    CHR     chrname;
    START   start;
    END     end;
    STAND   strand;
    FLANK   flankseq;
    A       allele_a;
    B       allele_b;
    AnnotCsvEntry(
          PSI  &&   probeset_id
        , CHR  &&   chrname
        , START&&   start
        , END  &&   end
        , STAND&&   strand
        , FLANK&&   flankseq
        , A    &&   allele_a
        , B    &&   allele_b
    )
    : probeset_id  (std::move(probeset_id    )) 
    , chrname      (std::move("chr" + chrname))  
    , start        (std::move(start - 1      ))  
    , end          (std::move(end            ))  
    , strand       (std::move(strand         ))  
    , flankseq     (std::move(flankseq       ))  
    , allele_a     (std::move(allele_a       ))  
    , allele_b     (std::move(allele_b       ))  
    {}
};

using AnnotCsvParseTraitProto = cpt::format::TraitPrototype<
    AnnotCsvEntry
    , cpt::format::PMT<0 , std::string> // probe set id
    , cpt::format::PMT<4 , std::string> // chromosome
    , cpt::format::PMT<5 , uint64_t>    // start ( -1)
    , cpt::format::PMT<6 , uint64_t>    // end
    , cpt::format::PMT<7 , char>        // strand
    , cpt::format::PMT<10, std::string> // flank seq
    , cpt::format::PMT<11, std::string> // Allele A
    , cpt::format::PMT<12, std::string> // Allele B
>;

struct AnnotCsvParseTrait
{
    using Entry = AnnotCsvParseTraitProto::Entry;
    using TakeRowPos = AnnotCsvParseTraitProto::TakeRawPos;
    using FieldTypes = AnnotCsvParseTraitProto::FieldTypes;
    template<class FT, int POS, class STR>
    static auto field_parse(STR&& str)
    {
        return boost::lexical_cast<FT>(
            boost::make_iterator_range( 
                  str.begin() + 1
                , str.end()   - 1
            )
        );
    }
    const static std::string token_string;
};
// #ifdef SINGLE_CPP
// const std::string AnnotCsvParseTrait::token_string = ",";
// #endif

using AnnotCsvParser = cpt::format::TupleParser<AnnotCsvParseTrait>;

struct AnnotCsv : public std::vector< typename AnnotCsvParseTrait::Entry >
{
    std::map<std::experimental::string_view, std::size_t> probeset_id_idx;
    template<class IS>
    void read( IS&& is )
    {
        namespace ba = boost::adaptors;
        AnnotCsvParser parser;
        std::string line;
        while( std::getline(is, line))
        {
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
    auto&& get_entry( const std::experimental::string_view& probeset_id )
    {
        return this->at(probeset_id_idx.at(probeset_id));
    }
};

}}}}
// MAKE_LTUPLE_HEADER(cpt::application::cdf2cad::axiom::AnnotCsvIdx
//     , "probe_set_id"
//     , "chrname"
//     , "start"
//     , "end"
//     , "strand"
//     , "flank"
//     , "allele A"
//     , "allele B"
// );
