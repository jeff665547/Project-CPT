#pragma once
#include <CPT/engine/data_pool/component_object_manager/ctx_obj_type.hpp>
#include <functional>
namespace cpt { namespace engine { namespace data_pool { namespace component_object_manager {
namespace detail {

struct DataAndTypeDeleter
{
    using DyDataDeleter = std::function<void(void*&)>;
    DyDataDeleter* dy_data_deleter;
    void** data;
    // DataAndTypeDeleter() = delete;
    DataAndTypeDeleter( DyDataDeleter& deleter, void*& _data )
    : dy_data_deleter ( &deleter )
    , data ( &_data )
    {
        // std::cout << data << '\t' << "deleter construct(null)" << std::endl;
    }
    // DataAndTypeDeleter( const DataAndTypeDeleter& d )
    // : dy_data_deleter( d.dy_data_deleter )
    // , data( d.data )
    // {
    //     std::cout << data << '\t' << "copy construct(null)" << std::endl;
    // }
    // DataAndTypeDeleter( DataAndTypeDeleter&& d )
    // : dy_data_deleter( d.dy_data_deleter )
    // , data( d.data )
    // {
    //     std::cout << data << '\t' << "move construct(null)" << std::endl;
    // }
    void operator() ( CtxObjType* ptr )
    {
        if ( ((bool)(*dy_data_deleter)) && data != nullptr)
            dy_data_deleter->operator()( *data );
        delete ptr;
        ptr = nullptr;
    }
    // ~DataAndTypeDeleter()
    // {
    //     std::cout << data << '\t' << "deleter destruct" << std::endl;
    // }
};
template<class T>
auto make_deleter ()
{
    return [](void*& p)
    {
        delete (T*)p;
    };
}

}}}}}
