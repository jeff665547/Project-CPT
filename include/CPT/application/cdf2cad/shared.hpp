#pragma once
#include <string>
#include <CPT/application/cdf2cad/main.hpp>
// #include <CPT/dll_config.hpp>
namespace cpt { namespace application { 
namespace cdf2cad {
void file_convert_to(
      const std::string& input    
    , const std::string& output   
    , const std::string& probe_tab
    , const std::string& annot_csv
)
{
    cpt::application::cdf2cad::Parameters params
    {
          input    
        , output   
        , probe_tab
        , annot_csv
    };
    // params.path_to_cdf      ( input  );
    // params.path_to_cad      ( output );
    // params.path_to_probe_tab( probe_tab );
    // params.path_to_annot_csv( annot_csv );
    cpt::format::converter::AxiomCdf2CadConverter converter;
    converter( 
          params.path_to_cdf
        , params.path_to_cad
        , params.path_to_probe_tab
        , params.path_to_annot_csv
    );
}
}}}
