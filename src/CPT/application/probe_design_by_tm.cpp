#include <CPT/exception.hpp>
#include <CPT/exception/std_exception_json_dump.hpp>
#include <CPT/config_reader/config_reader.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/application/probe_design_by_tm/data_pool.hpp>
#include <CPT/application/probe_design_by_tm/probe_design_by_tm.hpp>

namespace cpt {
namespace application {
namespace probe_design_by_tm {

class OptionParser : public cpt::OptionParser
{
public:
    std::ifstream json_config;
    std::ofstream output_file;

    OptionParser(int argc, char const * argv[])
    {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show help message")
            ("input,i", po::value<std::string>(), "input json file")
            ("output,o", po::value<std::string>(), "output file")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if(vm.count("help"))
        {
            std::cout << desc << "\n";
            exit(1);
        }
        std::string input = "";
        std::string output = "";
        get_parameter("input", input);
        get_parameter("output", output);
        json_config.open ( input == "" ? "/dev/stdin" : input );
        output_file.open ( output == "" ? "/dev/stdout" : output );
    }
};

namespace bpt = boost::property_tree;
template<class OPTION_PARSER>
class Main
{
    OPTION_PARSER args_;
    cpt::application::probe_design_by_tm::DataPool data_pool_;

  public:
    Main( OPTION_PARSER&& args )
    : args_( std::forward<OPTION_PARSER>( args ) )
    , data_pool_( args_ )
    {}
    void operator()()
    {
        cpt::application::probe_design_by_tm::ProbeDesignByTm probe_design_by_tm( data_pool_ );
        probe_design_by_tm();
    }
};

template<class OPTION_PARSER>
Main<OPTION_PARSER> make ( OPTION_PARSER&& option_parser )
{
    return Main<OPTION_PARSER> ( 
      std::forward < OPTION_PARSER > ( option_parser )
    );
}

} // probe_design_by_tm
} // application
} // cpt

int main ( int argc, const char* argv[] )
{
    cpt::application::probe_design_by_tm::OptionParser option_parser( argc, argv );
    auto&& probe_design_by_tm ( cpt::application::probe_design_by_tm::make( option_parser ));
    probe_design_by_tm();
    return 0;
}
