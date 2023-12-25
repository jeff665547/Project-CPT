#pragma once
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
namespace cpt { namespace utility {

template<class ARCHIVE>
struct SerialOwrap
{
    template<class T>
    static decltype(auto) call( T&& o )
    {
        return std::forward<T>(o);
    }
};
template<class T>
decltype(auto) xml_serial_owrap( T&& o )
{
    return BOOST_SERIALIZATION_NVP( std::forward<T>(o) );
}
template<>
struct SerialOwrap<boost::archive::xml_iarchive> 
{
    template<class T>
    static decltype(auto) call( T&& o )
    {
        return xml_serial_owrap(std::forward<T>(o));
    }
};
template<>
struct SerialOwrap<boost::archive::xml_oarchive> 
{
    template<class T>
    static decltype(auto) call( T&& o )
    {
        return xml_serial_owrap(std::forward<T>(o));
    }
};

}}
