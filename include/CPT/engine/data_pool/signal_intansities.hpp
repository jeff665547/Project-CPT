#pragma once
#include <CPT/forward.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Nucleona/language.hpp>
#include <armadillo>
#include <vector>

namespace cpt {
namespace engine {
namespace data_pool {

class SignalIntansitiesImpl
{

  public:

    std::vector< std::vector< float >> signal_intansities_[2];

};
using SignalIntansities = SignalIntansitiesImpl;

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
