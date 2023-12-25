#pragma once
#include <utility>
#include <CPT/format/tsv.hpp>
#include <CPT/format/static_tsv_reader.hpp>
namespace cpt {
namespace utility {
namespace cf = cpt::format;
// namespace ced = cpt::engine::data_pool;

template<class DB>
struct AlleleSignalLoader
{
    using AlleleSignalsTrait = cpt::format::TraitPrototype<
          std::tuple
        , cf::PMT<0, std::string>
        , cf::PMT<1, double>
        , cf::PMT<2, double>
    >;
    using AlleleSignalsParser = cf::TupleParser<AlleleSignalsTrait>;
    DB db;
    AlleleSignalLoader(DB&& db)
    : db( std::forward<DB>( db ) )
    {}

    std::vector< cf::Tsv<> > operator()() 
    {
        AlleleSignalsParser parser;
        std::vector< cf::Tsv<> > res;
        auto& paths ( db.get_path_list("allele_signals") );
        for( auto& p : paths )
        {
            cf::Tsv<> tsv;
            tsv.register_column("probeset_id");
            tsv.register_column("allele_a");
            tsv.register_column("allele_b");
            std::ifstream is ( p.string() );
            std::string line;
            while ( std::getline( is, line ) )
            {
                auto tup ( parser(line) );
                tsv.push_entry( 
                      std::move(std::get<0>(tup))
                    , std::move(std::get<1>(tup))
                    , std::move(std::get<2>(tup))
                );
            }
            res.emplace_back(std::move(tsv));
        }
        return res;
    }
};
template<class DB>
auto make_allele_signal_loader( DB&& db )
{
    return AlleleSignalLoader<DB>(std::forward<DB>(db));
}

}}
