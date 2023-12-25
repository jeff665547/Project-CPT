#pragma once
#include <memory>
#include <CPT/engine/data_pool/component_object_manager/ctx_obj_type.hpp>
#include <CPT/engine/data_pool/component_object_manager/detail/type_resolver.hpp>
#include <CPT/engine/data_pool/component_object_manager/detail/type_checker.hpp>
#include <CPT/engine/data_pool/component_object_manager/detail/data_deleter.hpp>
#include <functional>
#include <CPT/utility.hpp>
namespace cpt { namespace engine { namespace data_pool {
namespace component_object_manager {

struct CPSymTabIns
{
    void* data;
    std::shared_ptr<CtxObjType> type_info;

    template<class T>
    CPSymTabIns( 
          T* _data
        , CtxObjType* _type_info
    )
    : data ( _data )
    , type_info ( 
          _type_info
    )
    {
        update_type_info_destruct_event<T>();
    }
    CPSymTabIns( 
        CtxObjType* _type_info
    )
    : data ( nullptr )
    , type_info ( 
          _type_info
    )
    {}
    template<class T>
    void update_type_info_destruct_event()
    {
        if ( type_info ) 
        {
            auto t_data = this->data;
            type_info->destruct_event = [t_data]()
            {
                if ( t_data )
                    delete (T*)(t_data);
            };
        }
    }
    template<class T>
    void initialize()
    {
        detail::TypeResolver<T>::get_ref(data, type_info);
        update_type_info_destruct_event<T>();
    }
    template< class T >
    void initialize( T&& o )
    {
        if ( data == nullptr && type_info->id == CtxObjType::ADDR )
        {
            data = new std::decay_t<T>(std::forward<T>(o));
            update_type_info_destruct_event<T>();
        }
        else
        {
            throw std::logic_error("initialize fail : constrain check fail");
        }
    }
    template< class T >
    void initialize_or( T&& o )
    {
        bool data_is_null = ( data == nullptr );
        bool type_is_addr_type = ( type_info->id == CtxObjType::ADDR );
        if ( data_is_null && type_is_addr_type )
        {
            data = new std::decay_t<T>(std::forward<T>(o));
            update_type_info_destruct_event<T>();
        }
        else if ( data_is_null && !type_is_addr_type )
        {
            throw std::logic_error("initialize fail : constrain check fail");
        }
        else if ( !data_is_null && type_is_addr_type )
        {
            return;
        }
        else
        {
            initialize<std::decay_t<T>>();
        }
    }
    template<class T>
    T& get()
    {
        if ( detail::TypeChecker<T>::check( type_info ) )
        {
            return *((T*)data);
        }
        else
        {
            throw std::logic_error("dynamic type check fail!!!");
        }
    }
    template<class T>
    void show_info(T&& out, int il = 0)
    {
        out << cpt::indent(il) << "data addr : " << data << std::endl;
        out << cpt::indent(il) << "ref count : " << type_info.use_count() << std::endl;
        out << cpt::indent(il) << "typeinfo addr : " << &(*type_info) << std::endl;
        out << cpt::indent(il) << "typeinfo : " << std::endl;
        type_info->show_info(out, il + 1); 
    }
    // ~CPSymTabIns()
    // {
    //     std::cout << "destruct : " << std::endl;
    //     show_info( std::cout );
    // }
};
}}}}
