#include <string>
#include <boost/python.hpp>
#include <CPT/application/cadtool/main.hpp>
#include <CPT/application/cadtool/shared.hpp>
namespace py_ = boost::python;
using namespace std::string_literals;
using namespace cpt::application::cadtool;

BOOST_PYTHON_FUNCTION_OVERLOADS( json2cad_file_convert_to_ol   , json2cad_file_convert_to, 2, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( cad2json_file_convert_to_ol   , cad2json_file_convert_to, 2, 2 )
BOOST_PYTHON_FUNCTION_OVERLOADS( file_convert_to_ol            , file_convert_to, 3, 3 )
BOOST_PYTHON_MODULE(MODULE_NAME)
{
    py_::def(
          "json2cad_file_convert_to"
        , &json2cad_file_convert_to
        , json2cad_file_convert_to_ol((
            py_::arg("input_file_path"),
            py_::arg("output_file_path") 
        ))
    );
    py_::def(
          "cad2json_file_convert_to"
        , &cad2json_file_convert_to
        , cad2json_file_convert_to_ol((
            py_::arg("input_file_path"),
            py_::arg("output_file_path") 
        ))
    );
    py_::def(
          "file_convert_to"
        , &file_convert_to
        , file_convert_to_ol((
            py_::arg("input_file_path"),
            py_::arg("output_file_path") 
        ))
    );
}
