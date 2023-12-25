#pragma once
#include <exception>
#include <iostream>
#include <CPT/exception/error_info.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace cpt
{

class Exception : virtual public std::exception
{
  protected:
      cpt::exception::ErrorInfo error;
  public:
    Exception ( const std::string& msg )
    {
        error.set_message( msg );
    }
    virtual void json_dump( std::ostream& out ) const 
    {
        error.json_dump(out);
    }
};

}
