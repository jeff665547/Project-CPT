#pragma once
#include <CCD/analyzer/probe_tm.hpp>
#include <CPT/application/probe_design_by_tm/data_pool.hpp>
#include <CPT/application/probe_design_by_tm/axiom_type.hpp>
#include <CPT/application/probe_design_by_tm/snp6_type.hpp>
#include <ctime>
#include <chrono>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

class ProbeDesignByTm
{
  public:

    cpt::application::probe_design_by_tm::DataPool& db;

    template< class DP >
    ProbeDesignByTm( DP& dp )
    : db( dp )
    {}

    void operator()()
    {
        auto start = std::chrono::time_point< std::chrono::system_clock >( std::chrono::system_clock::now() );

        if( db.method_type == "Axiom" )
        {
            AxiomType run( db );
            run();
        }
        else if( db.method_type == "SNP6" )
        {
            Snp6Type run( db );
            run();
        }
        else
        {
            std::runtime_error( "method_type most be \"Axiom\" or \"SNP6\"" );
        }

        db.output_probe_res( db.is_probecheck );

        auto end = std::chrono::time_point< std::chrono::system_clock >( std::chrono::system_clock::now() );
        db.output_file << std::chrono::duration< double >( end - start ).count() << "\n";
    }
};

} // probe_design_by_tm
} // application
} // cpt
