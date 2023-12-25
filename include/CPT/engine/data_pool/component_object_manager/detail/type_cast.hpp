#pragma once
#include <stdexcept>
namespace cpt { namespace engine { namespace data_pool { namespace component_object_manager {
namespace detail {

template<class T, class I>
T type_cast( I&& i )
{
    throw std::logic_error("not implement error");
    // return (T)i;
}
template<class T>
T type_cast( T&& i )
{
    return i;
}

}}}}}
