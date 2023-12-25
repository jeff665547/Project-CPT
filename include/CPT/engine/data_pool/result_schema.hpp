#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Nucleona/language.hpp>
#include <CPT/forward.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class ResultSchemaImpl
{
  protected:
    bpt::ptree result_schema_;
    std::ostream* os_;

  public:
    ResultSchemaImpl( std::ostream& os )
    : os_( &os )
    {
        bpt::ptree empty_tree;
        result_schema_.add_child("output", empty_tree);
    }
    GETTER ( bpt::ptree&, result_schema(),
    {
        return result_schema_;
    })
    void write_result_schema() const 
    {
        bpt::write_json( (*os_), result_schema_, true);
    }
    void add_output_path(bpt::ptree& node, const bfs::path& path)
    {
        bpt::ptree leaf;
        leaf.put_value(path.string());
        node.push_back(std::make_pair("", leaf));
    }
    bpt::ptree& require_output(const std::string& path)
    {
        static auto& output = result_schema_.get_child("output");
        try
        {
            return output.get_child(path);
        }
        catch (const bpt::ptree_bad_path& e)
        {
            bpt::ptree empty_tree;
            return output.add_child(path, empty_tree);
        }
    }
};
using ResultSchema = ResultSchemaImpl;

} // namespace data_pool
} // namespace engine
} // namespace cpt
