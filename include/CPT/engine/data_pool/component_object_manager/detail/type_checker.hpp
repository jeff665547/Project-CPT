#pragma once

namespace cpt { namespace engine { namespace data_pool { namespace component_object_manager {
namespace detail {
template<class T>
struct TypeChecker
{
    template<class TYPE_PTR>
    static bool check( TYPE_PTR p_type_info )
    {
        return true;
        /* TODO type check here */
    }
};

}}}}} 
