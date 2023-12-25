#pragma once
#include <CPT/forward.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Nucleona/language.hpp>
namespace cpt { namespace engine {
namespace data_pool
{

class PipelineSchemaImpl
{
  protected:
    bpt::ptree pipeline_schema_;
    bfs::path output_dir_;
  public:
    template<class Istream>
    PipelineSchemaImpl( Istream& is )
    {
        bpt::read_json( is, pipeline_schema_ );
        auto dir_str (
            pipeline_schema_
                #ifdef NEW_DATA_POOL
                .get_child("context.output_dir")
                #else
                .get_child("output_dir")
                #endif
                .get_value<std::string>()
        );
        output_dir_ = std::move( bfs::path( dir_str ) );
    }
    GETTER ( bpt::ptree&, pipeline_schema(),
    {
        return pipeline_schema_;
    })
    GETTER ( bfs::path&, output_dir(),
    {
        return output_dir_;
    })
};
using PipelineSchema = PipelineSchemaImpl;

}
}}
