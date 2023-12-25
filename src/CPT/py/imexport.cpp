#include <string>
#include <boost/python.hpp>
#include <CPT/application/imexport/shared.hpp>
namespace py_ = boost::python;
using namespace std::string_literals;
// void improc(
//     bool gui
//     , 
BOOST_PYTHON_MODULE(MODULE_NAME)
{
    py_::def(
          "json2cad_file_convert"
        , &json2cad_file_convert
        , json2cad_file_convert_ol((
            py_::arg("input_file_path"),
            py_::arg("output_file_path") 
        ))
    );
    py_::def(
          "cad2json_file_convert"
        , &cad2json_file_convert
        , cad2json_file_convert_ol((
            py_::arg("input_file_path"),
            py_::arg("output_file_path") 
        ))
    );
    py_::def(
          "file_convert"
        , &file_convert
        , file_convert_ol((
            py_::arg("input_file_path"),
            py_::arg("output_file_path") 
        ))
    );
}
