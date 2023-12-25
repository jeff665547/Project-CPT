#pragma once
#include <CPT/forward.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Nucleona/language.hpp>
#include <armadillo>
#include <vector>
namespace cpt
{
namespace engine
{
namespace data_pool
{

class SignalIntansitiesImpl
{
  public:
    std::vector<std::vector<float>> signal_intansities[2];
};
using SignalIntansities = SignalIntansitiesImpl;

// template <class CUBE>
// class SignalIntansitiesImpl
// {
//   public:
//     CUBE cube; // dims = [ probe, sample, channel ]
// };

}
}
}
