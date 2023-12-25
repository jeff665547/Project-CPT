#include <string>
#include <boost/python.hpp>
#include <CPT/application/cenfile_builder/shared.hpp>

namespace py_ = boost::python;
using namespace std::string_literals;

void build(
      const std::string& schema_json
    , const std::string& result_cenfile_hdf5 
)
{
    cpt::application::cenfile_builder::build(
        schema_json, result_cenfile_hdf5
    );
}

BOOST_PYTHON_FUNCTION_OVERLOADS( build_ol, build, 2, 2 )
BOOST_PYTHON_MODULE(MODULE_NAME)
{
    py_::def(
          "build"
        , &build
        , build_ol((
              py_::arg("schema_json")
            , py_::arg("result_cenfile_hdf5") 
        ))
    );
}
