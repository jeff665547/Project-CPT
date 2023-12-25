#pragma once
#include <CPT/config_reader/config_reader.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/format/converter.hpp>
#include <CPT/application/cadtool/shared.hpp>
namespace cf = cpt::format;
namespace cpt {
namespace application {
namespace cadtool {
class OptionParser : public cpt::OptionParser, public Parameters
{
public:
    OptionParser(int argc, char const * argv[])
    {
        po::options_description desc( "Allowed options" );
        desc.add_options()
            ( "help,h"  , "show help message" )
            ( "input,i" , po::value<std::string>(), "input file" )
            ( "output,o", po::value<std::string>(), "output file" )
            ( "mode,m"  , po::value<std::string>(), "mode (json2cad/cad2json)" )
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if(vm.count("help") || argc <= 1)
        {
            cpt::msg << desc << std::endl;
            exit(1);
        }
        std::string   mode( "" );

        get_parameter( "input" , ipath );
        get_parameter( "output", opath );
        get_parameter( "mode"  , mode );

        try { converter = std::move ( get_converter_creater( mode ) );  }
        catch( const std::exception& e )
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }
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
        // cpt::application::cadtool::Json2Cad j2c;
        // j2c.operator()( args_.ipath, args_.opath );
        args_.converter->operator()(args_.ipath, args_.opath);
        file_convert_to_impl( 
            args_
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
