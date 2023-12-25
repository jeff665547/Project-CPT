#pragma once
#include <CPT/forward.hpp>
#include <CPT/config_reader/config_reader.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/format/converter/cdf2cad_converter.hpp>
namespace cpt {
namespace application {
namespace cdf2cad {

struct Parameters
{
    std::string path_to_cdf;
    std::string path_to_cad;
    std::string path_to_probe_tab;
    std::string path_to_annot_csv;
};

class OptionParser
    : public cpt::OptionParser
    , public Parameters
{
public:
    OptionParser(int argc, char const * argv[])
    {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h"       , "show help message")
            ("input,i"      , po::value<std::string>()->required()       , "cdf file")
            ("output,o"     , po::value<std::string>()->required()       , "cad file")
            ("probe_tab,p"  , po::value<std::string>()->default_value(""), "probe_tab file")
            ("annot_csv,a"  , po::value<std::string>()->default_value(""), "annot_csv file")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(argc == 1 or vm.count("help"))
        {
            cpt::msg << desc << std::endl;
            exit(1);
        }
        po::notify(vm);
        get_parameter ("input"     , path_to_cdf);
        get_parameter ("output"    , path_to_cad);
        get_parameter ("probe_tab" , path_to_probe_tab);
        get_parameter ("annot_csv" , path_to_annot_csv);
    }
};

template<class OPTION_PARSER>
class Main
{
    OPTION_PARSER args_;
  public:
    Main( OPTION_PARSER&& args )
    : args_( std::forward<OPTION_PARSER>( args ) )
    {}

    void operator()()
    {
        cpt::format::converter::AxiomCdf2CadConverter converter;
        converter( 
              args_.path_to_cdf
            , args_.path_to_cad
            , args_.path_to_probe_tab
            , args_.path_to_annot_csv
        );
    }
};

template<class OPTION_PARSER>
auto make ( OPTION_PARSER&& option_parser )
{
    return Main<OPTION_PARSER> ( 
        std::forward < OPTION_PARSER > ( option_parser )
    );
}

}}}
