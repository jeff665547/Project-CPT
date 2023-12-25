#pragma once
#include <mlpack/core.hpp>
#include <CPT/utility/arma_debug_utility.hpp>
#include <CPT/config_reader/config_reader.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/engine/data_pool.hpp>
#include <CPT/engine/component.hpp>
#include <CPT/engine/pipeline.hpp>
#include <CPT/engine/component_factory.hpp>
#include <Nucleona/language.hpp>
#include <CPT/logger_spdlog.hpp>
namespace cpt
{
namespace application
{
namespace pipeline_builder
{
class OptionParser : public cpt::OptionParser
{
public:
    std::ifstream pipeline_schema_stream_;
    std::ofstream result_schema_stream_;

    OptionParser(int argc, char const * argv[])
    {
        int debug_level = 6;
        cpt::Log.set_level(std::min(debug_level, 6));
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "show help message")
            ("input,i", po::value<std::string>(), "input json file")
            ("output,o", po::value<std::string>(), "output json file")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if(vm.count("help") || argc <=1 )
        {
            std::cout << desc << "\n";
            exit(1);
        }
        std::string input = "";
        std::string output = "";
        get_parameter("input", input);
        get_parameter("output", output);
        pipeline_schema_stream_.open ( input == "" ? "/dev/stdin" : input );
        result_schema_stream_.open ( output == "" ? "/dev/stdout" : output );
    }
};

namespace bpt = boost::property_tree;
template<class OPTION_PARSER>
class Main
{
    OPTION_PARSER args_;
    cpt::engine::DataPool data_pool_;
  public:
    Main( OPTION_PARSER&& args )
    : args_( std::forward<OPTION_PARSER>( args ) )
    , data_pool_( args_ )
    {}
    DISABLE_COPY(Main);
    DEFAULT_MOVE(Main);
    void build_rule()
    {}
    auto build_pipeline()
    {
        std::vector<cpt::engine::ComponentPtr> res;
        cpt::engine::ComponentFactory component_factory ( data_pool_ );
        auto&& pipeline_schema ( data_pool_.pipeline_schema() ); 
        auto children ( pipeline_schema.get_child("pipeline") );
        for ( auto& child : children )
        {
            res.emplace_back( component_factory( child ) );
        }
        return cpt::engine::make_pipeline(std::move(res));
    }
    int operator()()
    {
        /* TODO if build rule up , then build rule 
         * otherwise build pipeline
         */
        auto pipeline ( build_pipeline() ) ;
        pipeline();
        data_pool_.write_result_schema();
        return 0;
    }
};

template<class OPTION_PARSER>
Main<OPTION_PARSER> make ( OPTION_PARSER&& option_parser )
{
    return Main<OPTION_PARSER> ( 
      std::forward < OPTION_PARSER > ( option_parser )
    );
}

}
}
}
