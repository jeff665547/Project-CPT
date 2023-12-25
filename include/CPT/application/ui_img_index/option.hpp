#pragma once
#include <CPT/forward.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/logger.hpp>
#include <CPT/utility/assert.hpp>
#include <CPT/utility/unique_ptr.hpp>
namespace cpt {
namespace application {
namespace ui_img_index {
namespace cf_   = cpt::format;
struct Parameters
{
    enum OutputMode { single, multi };

    OutputMode  mode            ; 
    std::string input_dir       ;
    std::string output_str      ;
    uint16_t    tile_width      ;
    uint16_t    tile_height     ;
    std::string tile_img_prefix ;
    std::string org_pos         ;
    std::string x_dirc          ;
};

class OptionParser
    : public cpt::OptionParser
    , public Parameters
{
public:
    OptionParser(int argc, char const * argv[])
    {
        po::options_description desc("Usage : output_dir and output_file are exculsive option, please choose one mode to use");
        desc.add_options()
            ("help,h"           , "show help message")
            ("tile_width,w"     , po::value<uint16_t>()->required()             , "tile width")
            ("tile_height,l"    , po::value<uint16_t>()->required()             , "tile height")
            ("input_dir,i"      , po::value<std::string>()->required()          , "input directory")
            ("output_dir,o"     , po::value<std::string>()                      , "output directory ( split file mode )")
            ("output_file,f"    , po::value<std::string>()                      , "output file ( single file mode )" )
            ("tile_img_prefix,t", po::value<std::string>()->required()          , "tile images output prefix" )
            ("x_dirc,x"         , po::value<std::string>()->required())
            ("org_pos,p"        , po::value<std::string>()->required())
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(argc == 1 || vm.count("help"))
        {
            cpt::msg << desc << std::endl;
            exit(1);
        }
        po::notify(vm);
        if ( vm.count("output_dir") && vm.count("output_file") )
        {
            cpt::fatal << "output_dir and output_file are exclusive option!" << std::endl;
            exit(1);
        }
        if ( !vm.count("output_dir") && !vm.count("output_file") )
        {
            cpt::fatal << "no output mode is set" << std::endl;
            exit(1);
        }
        if ( vm.count("output_dir") )
        {
            get_parameter ("output_dir"     , output_str    );
            mode = OutputMode::multi;
        }
        if ( vm.count("output_file") )
        {
            get_parameter ("output_file"    , output_str    );
            mode = OutputMode::single;
        }
        get_parameter ("input_dir"      , input_dir         );
        get_parameter ("tile_width"     , tile_width        );
        get_parameter ("tile_height"    , tile_height       );
        get_parameter ("tile_img_prefix", tile_img_prefix   );
    }
};
}}}
