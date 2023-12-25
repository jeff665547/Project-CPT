#include <CPT/option_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/application/improc2/improc.hpp>

namespace cpt {
namespace application {
namespace improc2 {

class OptionParser
    : public cpt::OptionParser
{
  public:
    std::string path_to_input;
    std::string path_to_output;
    std::string path_to_options;
    int32_t cv_num_threads;

    OptionParser(int argc, char* argv[])
    {
        po::options_description common("common options");
        common.add_options()

            ( "help,h"
            , "show this help messages")

            ( "cv-num-threads,j"
            , po::value(&cv_num_threads)->default_value(0)
            , "number of threads used by OpenCV")
            ;

        po::options_description io("input/output configuration");
        io.add_options()

            ( "input,i"
            , po::value<std::string>(&path_to_input)->required()
            , "file path to input file" )

            ( "output,o"
            , po::value<std::string>(&path_to_output)->required()
            , "file path to output file" )

            ( "options,s"
            , po::value<std::string>(&path_to_options)->required()
            , "file path to pipeline options" )
            ;

        po::options_description desc("Allow options");
        desc.add(common)
            .add(io)
            ;

        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (argc == 1 or vm.count("help"))
        {
            cpt::msg << desc << std::endl;
            exit(1);
        }
        po::notify(vm);
    }
};

template <class OPTION_PARSER>
class Main
{
  private:
    OPTION_PARSER args_;

  public:
    Main(OPTION_PARSER&& args)
      : args_(std::forward<OPTION_PARSER>(args))
    {}
    void operator()(void)
    {
        cpt::application::improc2::Improc improc(
            args_.path_to_options
          , args_.cv_num_threads
        );
        improc.run(
            args_.path_to_input
          , args_.path_to_output
        );
    }
};

template <class OPTIONS_PARSER>
auto make(OPTIONS_PARSER&& option_parser)
{
    return Main<OPTIONS_PARSER>(
        std::forward<OPTIONS_PARSER>(option_parser)
    );
}

} // namespace improc2
} // namespace application
} // namespace cpt
