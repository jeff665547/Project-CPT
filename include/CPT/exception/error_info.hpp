#pragma once
#include <boost/property_tree/ptree.hpp>
#include <CPT/forward.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace cpt
{
namespace exception
{

struct ErrorInfo
{
    bpt::ptree content; 
    ErrorInfo()
    {
        content.put( "status", "error" );
    }
    void set_message ( const std::string& str )
    {
        content.put ( "msg", str );
    }
    void json_dump( std::ostream& out ) const 
    {
        boost::property_tree::write_json(
              out
            , content
#ifdef DEBUG
            , true
#else
            , false
#endif
        );
    }
};

}
}
