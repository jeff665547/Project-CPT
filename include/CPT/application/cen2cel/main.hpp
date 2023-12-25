#pragma once
#include <CPT/forward.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/format/converter/array2cel_converter.hpp>
#include <CPT/format/chip_sample.hpp>
#include <CPT/utility/assert.hpp>
#include <CPT/format/cen.hpp>
namespace cpt {
namespace application {
namespace cen2cel {
namespace cf_   = cpt::format;
namespace cfcs_ = cf_::chip_sample;
namespace cfc_  = cf_::converter;
struct Parameters
{
    std::string path_to_cen;
    std::string path_to_cel;
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
            ("input,i"      , po::value<std::string>()->required()       , "cen file (.cen)")
            ("output,o"     , po::value<std::string>()->required()       , "cel file (.cel)")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(argc == 1 or vm.count("help"))
        {
            cpt::msg << desc << std::endl;
            exit(1);
        }
        po::notify(vm);
        get_parameter ("input"     , path_to_cen     );
        get_parameter ("output"    , path_to_cel    );
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
        auto p_array = cpt::format::cen::File::load(args_.path_to_cen );
        auto& array = *((cpt::format::chip_sample::Array*)p_array.get());
        cfc_::Array2Cel array2cel;
        cpt::utility::safe_assert ( array2cel.operator()( array, args_.path_to_cel ) );
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
