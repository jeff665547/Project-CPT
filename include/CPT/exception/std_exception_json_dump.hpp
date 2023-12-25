#pragma once
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <exception>
#include <CPT/exception/error_info.hpp>
namespace cpt
{
namespace exception
{

void std_exception_json_dump ( const std::exception& e, std::ostream& out )
{
    ErrorInfo ei;
    ei.set_message(e.what());
    ei.json_dump(out);
}

}
}
