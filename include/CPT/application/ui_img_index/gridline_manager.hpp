#pragma once
#include <CPT/format/json.hpp>
#include <map>

namespace cpt { namespace application {
namespace ui_img_index {
namespace cf_ = cpt::format;
namespace bp_ = boost::property_tree;
struct ImgGridlineInfo
{
    std::vector<uint32_t> gl_x;
    std::vector<uint32_t> gl_y;
    uint32_t st_x;
    uint32_t st_y;
};
struct GridlineManager
{
    template<class T, class JL>
    auto to_vec( JL&& jl )
    {
        std::vector<T> res;
        for ( auto&& e : jl )
        {
           res.push_back( e.second.template get_value<T>() ); 
        }
        return res;
    };
    GridlineManager( const std::string& path )
    {
        auto jgridline_list( cf_::read_json( path ) );
        for ( auto&& p : jgridline_list.root )
        {
            auto jnode ( cf_::make_json(p.second) );
            auto org_pos ( to_vec<uint32_t>(jnode.get_list("org_pos")) );
            gridline_table_.emplace(
                p.first
                , ImgGridlineInfo
                {
                      to_vec<uint32_t>( jnode.get_list("gl_x") )
                    , to_vec<uint32_t>( jnode.get_list("gl_y") )
                    , org_pos.at(0)
                    , org_pos.at(1)
                }
            );
        }

    }
    auto& operator()( const std::string& id )
    {
        return gridline_table_.at(id);
    }
  private:
    std::map<
          std::string
        , ImgGridlineInfo
    > gridline_table_;
};
}}}
