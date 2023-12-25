#pragma once
#include <CPT/utility/ltuple.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <experimental/string_view>
#include <map>
#include <boost/range/adaptor/indexed.hpp>
// /CPT/src/application/cdf2cad/axiom
namespace cpt {
namespace format {
namespace axiom { 

namespace annot_csv_detail {
template<class T>
struct Default{};
#define DEFAULT(T, v) \
template<> struct Default<T> { static T value() { return v; } };
DEFAULT(std::string , ""    )
DEFAULT(uint64_t    , 0     )
DEFAULT(char        , '.'   )
} // namespace annot_csv_detail

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
    , cpt::format::PMT<11, char> // Allele A
    , cpt::format::PMT<12, char> // Allele B
>;

struct AnnotCsvParseTrait
{
    using Entry = AnnotCsvParseTraitProto::Entry;
    using TakeRawPos = AnnotCsvParseTraitProto::TakeRawPos;
    using FieldTypes = AnnotCsvParseTraitProto::FieldTypes;
    template<class FT, int POS, class STR>
    static auto field_parse(STR&& str)
    {
        std::experimental::string_view content((&*str.begin()), std::distance(str.begin(), str.end()));
        if ( content == "\"---\"" ) return annot_csv_detail::Default<FT>::value();
        return boost::lexical_cast<FT>(
            boost::make_iterator_range( 
                  str.begin() + 1
                , str.end()   - 1
            )
        );
    }
    const static std::string token_string;
};
#ifdef SINGLE_CPP
const std::string AnnotCsvParseTrait::token_string = ",";
#endif

using AnnotCsvParser = cpt::format::TupleParser<AnnotCsvParseTrait>;

struct AnnotCsv : public std::vector< typename AnnotCsvParseTrait::Entry >
{
    using Entry = typename AnnotCsvParseTrait::Entry;
    std::map<std::experimental::string_view, std::size_t> probeset_id_idx;
    template<class IS>
    void read( IS&& is )
    {
        namespace ba = boost::adaptors;
        AnnotCsvParser parser("\t");
        std::string line;
        bool header = false;
        while( std::getline(is, line))
        {
            if ( line[0] == '#' ) continue;
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
        auto itr( probeset_id_idx.find ( probeset_id ) );
        if ( itr == probeset_id_idx.end() ) return nullptr;
        else 
        {
            return &(this->at(itr->second));
        }
    }
    bool is_solid( const Entry& entry )
    {
        using namespace annot_csv_detail;
        if (   
               entry.end      == Default<uint64_t   >::value()
            && entry.flankseq == Default<std::string>::value()
            && entry.allele_a == Default<char       >::value()
            && entry.allele_b == Default<char       >::value()
        )
            return false;
        else return true;
    }
};

}}}
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
