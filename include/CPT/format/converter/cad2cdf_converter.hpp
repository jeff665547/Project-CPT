#pragma once
#include <CPT/format/cad.hpp>
#include <affy/calvin_files/writers/src/CDFFileWriter.h> 
#include <affy/calvin_files/data/src/CDFData.h>
#include <affy/calvin_files/data/src/CDFDataTypes.h>
#include <CPT/logger.hpp>
#include <CPT/utility/unique_ptr.hpp>
#include <CPT/format/converter/wstring_conv.hpp>
#include <CCD/utility/sequence_operation.hpp>
#include <CPT/format/json.hpp>
namespace cpt{ namespace format{ namespace converter{
namespace aci_ = affymetrix_calvin_io;
namespace cu_  = cpt::utility;

class Cad2CdfConverter
{
  public:
    auto get_unit_type( const std::string& ps_type )
    {
        // For Bioconductor
        // Expression 0
        // Genotyping 1 
        // Tag 2
        // Resequensing 3
        // ... etc.
        if ( ps_type == "Expression" )
        {
            return 0;
        }
        else if ( ps_type == "Genotyping" )
        {
            return 1;
        }
        else
        {
            throw std::runtime_error("unsupport type : " + ps_type );
        }

    }
    auto get_direction( char strand )
    {
        if ( strand == '+' )
            return 1;
        else if ( strand == '-' )
            return 2;
        else
        {
            return 3;
        }
    }
    auto operator()( const std::string& cadpath, const std::string& cdfpath )
    {
        WStrUTF8 code_conv;

        Cad cad;
        cad.read_cad( cadpath );

        std::unique_ptr<aci_::CDFData> cdf ( new aci_::CDFData(cdfpath) );
        cdf->SetArrayCols( cad.num_cols() );
        cdf->SetArrayRows( cad.num_rows() );
        // cpt::warn << "Warn : CDF data set temporary support Expression probeset type only" << 
        //     "assume all probeset is expression
        cdf->SetProbeSetCnt( cad.num_probesets(), aci_::Expression );
        aci_::CDFFileWriter cdfwtr (*cdf);
        auto&& cad_probeset_list = cad.get_probeset_list();
        for ( auto i = cad_probeset_list.size(); i > 0; i -- )
        {
            // if ( i != cad_probeset_list.size() ) break;
            auto&& cad_ps = cad_probeset_list.at(cad_probeset_list.size() - i);
            assert( cad_ps.name().length() <= 50 );
            cdfwtr.OpenDataGroup(code_conv.from_bytes(
                cad_ps.name()
            ), 1 );
            auto pswtr = cu_::make_unique( cdfwtr.CreateProbeSetWriter(
                  L"NONE"
                , get_unit_type( cad_ps.type() )
                , get_direction( cad_ps.strand() )
                // , cad_ps.num_probes() > 4 ? 4 : cad_ps.num_probes()
                // , cad_ps.num_probes() > 4 ? 4 : cad_ps.num_probes()
                , cad_ps.num_probes()
                , cad_ps.num_probes()
                , 0
                , 1
            ));
            pswtr->WriteHeader();
            auto&& cad_probe_list = cad_ps.get_probe_list();
            for ( auto j = cad_probe_list.size(); j > 0; j -- )
            {
                auto index = cad_probe_list.size() - j;
                // if ( index > 4 ) break;
                // std::cout << "index : " << index << std::endl;
                auto&& cad_probe 
                    = cad_probe_list.at( index );
                auto&& rs = cad_probe.region_des();
                auto&& jseq = cpt::format::make_json( cad_probe.sequence() );
                auto&& seq_cont = jseq.template get<std::string>("content");
                pswtr->Write(
                      std::get<0>( rs ) // X
                    , std::get<1>( rs ) // Y
                    , index             // atom
                    , 16                // index pos
                    , seq_cont.back()               // base probe
                    , complement( seq_cont.back() ) // base target
                );
            }
            pswtr->Close();
            pswtr.reset();
            cdfwtr.CloseDataGroup();
        }
        return true;
    }
};

}}}
