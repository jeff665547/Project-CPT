#pragma once
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <CPT/engine/data_pool/component_object_manager/ctx_obj_type.hpp>
#include <set>
#include <list>
#include <functional>
#include <CPT/engine/data_pool/component_object_manager/cpsym_tab_ins.hpp>
#include <CPT/format/json.hpp>
#include <CPT/utility.hpp>
#include <CPT/utility/language.hpp>

namespace cpt { namespace engine { namespace data_pool {
namespace component_object_manager {
class CPSymTab 
{
  public:
    using Entry = CPSymTabIns;
  private:
    static std::map< std::string, std::shared_ptr<CPSymTabIns>> data;
    static std::map< std::string, std::weak_ptr<CPSymTabIns>> data_view;
  public:
    // template<class T>
    static std::shared_ptr<CPSymTabIns> get_entry( 
        const std::string& ref_name 
    )
    {
        auto itr = data.find(ref_name);
        if ( data.end() == itr )
        {
            // throw std::logic_error("symbol table missing data" );
            return data.emplace_hint( itr, ref_name
                , std::shared_ptr<CPSymTabIns>(
                    new CPSymTabIns (
                        new CtxObjType { CtxObjType::ADDR }
                    )
                )
            )->second;
        }
        else
        {
            return itr->second;
        }
    }
    static auto get_tab_ins( const boost::property_tree::ptree& node )
    {
        if ( node.size() > 0 ) // check form
        {
            std::vector<std::string> path_list;
            for ( auto&& path : node )
            {
                path_list.emplace_back( path.second.data() );
            }
            return CPSymTabIns( 
                  new decltype(path_list)(std::move(path_list) )
                , new CtxObjType{ CtxObjType::PATH_LIST }
            );
                
        }
        else
        {
            return CPSymTabIns ( 
                  new std::string(node.data()) 
                , new CtxObjType{CtxObjType::PATH}
            );
        }
    }
    static void read_json_config( const boost::property_tree::ptree& context )
    {
        for ( auto&& spt : context )
        {
            data[spt.first] = std::shared_ptr<CPSymTabIns>(
                new CPSymTabIns( get_tab_ins ( spt.second ) )
                // new CPSymTabIns ( 
                //       new std::string(spt.second.data()) 
                //     , new CtxObjType{ get_ctx_obj_type( spt.second ) }
                //     // , new CtxObjType{CtxObjType::PATH}
                // )
            );
        }
        // show_info(std::cout);
    }
    template<class T>
    static void show_info(T&& out, int il = 0)
    {
        out << cpt::indent(il) << "solid symbol table : " << std::endl;
        for ( auto&& sym : data )
        {
            out << cpt::indent(il + 1) << sym.first << " : " << std::endl;
            out << cpt::indent(il + 2) << "symbol rc : " << sym.second.use_count() << std::endl;
            sym.second->show_info(out, il + 2);
        }
        out << cpt::indent(il) << "soft symbol table : " << std::endl;
        for ( auto&& sym : data_view)
        {
            out << cpt::indent(il + 1) << sym.first << " : " << std::endl;
            out << cpt::indent(il + 2) << "symbol rc : " << sym.second.use_count() << std::endl;
            if ( sym.second.expired() )
            {
                out << cpt::indent(il + 2) << "instance delete, no info" << std::endl;
            }
            else
            {
                auto sptr = sym.second.lock();
                sptr->show_info(out, il + 2);
            }
        }
    }
    static void initialize()
    {
        for ( auto&& e : data )
        {
            data_view.emplace( e.first, e.second );
        }
        data.clear();
    }
    static const auto& get_data()
    {
        return data;
    }
};
}}}}
