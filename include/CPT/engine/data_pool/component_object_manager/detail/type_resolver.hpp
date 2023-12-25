#pragma once
#include <CPT/engine/data_pool/component_object_manager/ctx_obj_type.hpp>
// #include <CPT/engine/data_pool/component_object_manager/cpsym_tab.hpp>
#include <stdexcept>
#include <CPT/utility/path_to_obj.hpp>
namespace cpt { namespace engine { namespace data_pool { namespace component_object_manager {

namespace detail {
template<class T>
struct TypeResolver
{
    template<class OREF, class TYPE>
    static void get_ref( 
          OREF&                 ref
        , TYPE&                 type
    )
    {
        if ( type->id == CtxObjType::PATH )
        {
            /* read data to T */
            std::string* path = (std::string*)ref;
            type->id = CtxObjType::ADDR;
            ref = new T( cpt::utility::PathToObj<T>::get( *path ) );
            delete path;
        }
        else if ( type->id == CtxObjType::PATH_LIST )
        {
            std::vector<std::string>* path_list = (std::vector<std::string>*)ref;
            type->id = CtxObjType::ADDR;
            
            ref = new T( cpt::utility::PathToObj<T>::get( *path_list ) );
            delete path_list;
        }
        else if ( type->id == CtxObjType::ADDR )
        {
            /* TODO get detail type info, need to do type cast */
            if ( ref == nullptr )
                ref = new T();
        }
        else if ( type->id == CtxObjType::KEY )
        {
            /* TODO reference again */
        }
        else
        {
            throw std::logic_error("unknown type detect\n");
        }
    }
};

}}}}}
