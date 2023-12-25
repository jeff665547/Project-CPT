#pragma once
#include <CPT/forward.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/format/converter/cad2cdf_converter.hpp>
#include <CPT/utility/assert.hpp>
namespace cpt {
namespace application {
namespace cad2cdf {
namespace cf_   = cpt::format;
namespace cfc_  = cf_::converter;
struct Parameters
{
    std::string path_to_cad;
    std::string path_to_cdf;
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
            ("input,i"      , po::value<std::string>()->required()       , "CAD file (.cad, .h5, .hdf5 )")
            ("output,o"     , po::value<std::string>()->required()       , "CDFfile (.cdf)")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(argc == 1 or vm.count("help"))
        {
            cpt::msg << desc << std::endl;
            exit(1);
        }
        po::notify(vm);
        get_parameter ("input"     , path_to_cad     );
        get_parameter ("output"    , path_to_cdf    );
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
        cfc_::Cad2CdfConverter cad2cdf;
        cpt::utility::safe_assert( cad2cdf.operator()( args_.path_to_cad, args_.path_to_cdf ) );
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
