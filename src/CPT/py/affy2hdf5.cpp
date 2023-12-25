#include <string>
#include <boost/python.hpp>
#include <CPT/application/affy2hdf5/shared.hpp>
namespace py_ = boost::python;
using namespace std::string_literals;

void file_convert_to(
      const std::string& affycel
    , const std::string& hdf5   
)
{
    cpt::application::affy2hdf5::file_convert_to(
        affycel, hdf5
    );
}

BOOST_PYTHON_FUNCTION_OVERLOADS( file_convert_to_ol, file_convert_to, 2, 2 )
BOOST_PYTHON_MODULE(MODULE_NAME)
{
    py_::def(
          "file_convert_to"
        , &file_convert_to
        , file_convert_to_ol((
            py_::arg("affycel"),
            py_::arg("hdf5") 
        ))
    );
}
