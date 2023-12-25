#include <boost/python.hpp>
#include <CPT/application/cdf2cad/shared.hpp>
namespace py_ = boost::python;
using namespace cpt::application::cdf2cad;
BOOST_PYTHON_FUNCTION_OVERLOADS( file_convert_to_ol, file_convert_to, 4, 4 )
BOOST_PYTHON_MODULE(MODULE_NAME)
{
    py_::def(
          "file_convert_to"
        , &file_convert_to
        , file_convert_to_ol((
            py_::arg("input")    ,
            py_::arg("output")   ,
            py_::arg("probe_tab"),
            py_::arg("annot_csv")
        ))
    );
}
