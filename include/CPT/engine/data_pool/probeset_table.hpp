#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
#include <CPT/format/cdffile.hpp>
#include <set>

namespace cpt {
namespace engine {
namespace data_pool {

class Probeset
{
  public:

    std::string probeset_id;
    std::string probeset_case;      // cases({ "AT", "CG", "NON" })

    std::vector< size_t > probes;   // probes id
    std::vector< char >   pbases;
    std::vector< char >   tbases;   // ignore p&t base
    std::vector< char >   alleles;
    std::vector< size_t > channels;

    size_t num_probes;

    Probeset()
        : probeset_id()
        , probeset_case()
        , probes()
        , pbases()
        , tbases()
        , alleles()
        , channels()
        , num_probes()
    {}

    std::string which_case_is()
    {
        std::vector< std::string > cases({ "AT", "CG", "OTHER" });

        std::set< char > pset;

        for( size_t i = 0; i < pbases.size(); ++ i )
        {
            pset.emplace(  pbases[i] );
        }

        if( pset.size() > 2 )
        {
            return cases[2];
        }
            
        bool AT = false;
        bool CG = false;

        for( auto&  p : pset )
        {
            if( p == 'A' || p == 'T' )
            {
                AT = true;
            }

            if( p == 'C' || p == 'G' )
            {
                CG = true;
            }
        }

        if( AT && !CG )
        {
            return cases[0];
        }
        else if( !AT && CG )
        {
            return cases[1];
        }
        else if( AT && CG )
        {
            return cases[2];
        }
        else
        {
            throw std::runtime_error(
                "Unknown case in probeset id = \"probeset_id\"\n"
            );
        }

        return "";
    }

    Probeset( cpt::format::CDFUnit& unit )
        : probeset_id( unit.name )
        , probeset_case()
        , probes()
        , pbases()
        , tbases()
        , alleles()
        , channels()
        , num_probes()
    {
        probes.reserve( unit.n_cells );
        pbases.reserve( unit.n_cells );
        tbases.reserve( unit.n_cells );
        alleles.reserve( unit.n_cells );
        channels.reserve( unit.n_cells );

        for( auto& block : unit.blocks )
        {
            for( auto& cell : block.cells )
            {
                probes.push_back( cell.id );
                pbases.push_back( cell.pbase );
                tbases.push_back( cell.tbase );
                channels.push_back(( size_t )( block.channel ));

                if( block.allele_code )
                {
                    alleles.push_back( 'B' );
                }
                else
                {
                    alleles.push_back( 'A' );
                }
            }
        }

        probeset_case = which_case_is();
        num_probes = probes.size();
    }

    Probeset( const Probeset& probeset )
        : probeset_id( probeset.probeset_id )
        , probeset_case( probeset.probeset_case )
        , probes( probeset.probes )
        , pbases( probeset.pbases )
        , tbases( probeset.tbases )
        , alleles( probeset.alleles )
        , channels( probeset.channels )
        , num_probes( probeset.num_probes )
    {}

    Probeset( Probeset&& probeset )
        : probeset_id( std::move( probeset.probeset_id ))
        , probeset_case( std::move( probeset.probeset_case ))
        , probes( std::move( probeset.probes ))
        , pbases( std::move( probeset.pbases ))
        , tbases( std::move( probeset.tbases ))
        , alleles( std::move( probeset.alleles ))
        , channels( std::move( probeset.channels ))
        , num_probes( std::move( probeset.num_probes ))
    {}

    Probeset& operator=( const Probeset& probeset )
    {
        probeset_id = probeset.probeset_id;
        probeset_case = probeset.probeset_case;
        probes = probeset.probes;
        pbases = probeset.pbases;
        tbases = probeset.tbases;
        alleles = probeset.alleles;
        channels = probeset.channels;
        num_probes = probeset.num_probes;
        return *this;
    }

    Probeset& operator=( Probeset&& probeset )
    {
        probeset_id = std::move( probeset.probeset_id );
        probeset_case = std::move( probeset.probeset_case );
        probes = std::move( probeset.probes );
        pbases = std::move( probeset.pbases );
        tbases = std::move( probeset.tbases );
        alleles = std::move( probeset.alleles );
        channels = std::move( probeset.channels );
        num_probes = std::move( probeset.num_probes );
        return *this;
    }
};

class ProbesetTable
{
  public:
    
    std::vector< Probeset > probeset_table;

    size_t num_probesets;
    size_t num_probes;

    ProbesetTable() = default;
    template< class DB >
    ProbesetTable( DB& db )
        : probeset_table()
        , num_probesets( 0 )
        , num_probes( 0 )
    {}
};

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
