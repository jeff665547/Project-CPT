#pragma once
#include <CPT/forward.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Nucleona/language.hpp>
#include <map>
namespace cpt
{
namespace engine
{
namespace data_pool
{

class DataPathsPoolImpl
{
  protected:
    std::map<
          std::string
        , std::vector<bfs::path>
    > data_path_;
  public:
    GETTER( bfs::path&, get_path(const std::string& name), 
    {
        return data_path_
            .at(name)
            .at(0);
    })
    GETTER ( std::vector<bfs::path>&, get_path_list ( const std::string& name ), 
    {
        return data_path_.at(name);
    })
    void push_path ( 
          const std::string& name
        , bpt::ptree& node )
    {
        data_path_[name].emplace_back ( 
            node.get_value<std::string>() );
    }
    void push_path ( std::pair<std::string, bpt::ptree>& inputs )
    {
        push_path ( inputs.first, inputs.second );
    }
    void push_path( 
          const std::string& name
        , const bfs::path& path
    )
    {
        data_path_[name].emplace_back(path);
    }
    // bool exist_path_tag( const std::string& name )
    // {
    //     auto dpitr = data_path_.find(name);
    //     if( dpitr != data_path_.end() ) 
    //     {
    //         if( dpitr->second.size() != 0 )
    //         {
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    void assign_path( const std::string& name, std::vector< bfs::path >& paths )
    {
        data_path_[ name ] = paths;
    }

    bool exist_path_tag( const std::string& tag )
    {
        auto data_path_it = data_path_.find( tag );

        if( data_path_it != data_path_.end() )
        {
            if( data_path_it->second.empty() )
            {
                return false;
            }

            return true;
        }

        return false;
    }

};
using DataPathsPool = DataPathsPoolImpl;

}
}
}
