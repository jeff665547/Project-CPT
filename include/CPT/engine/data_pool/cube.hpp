#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
#include <CPT/format/cube.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class Cube
{
  public:

    cpt::format::Cube< double > raw_sample_cube;
    cpt::format::Cube< double > cube;
    cpt::format::Cube< double > labelz_cube;

    template< class DB >
    Cube( DB& db )
        : raw_sample_cube()
        , cube()
        , labelz_cube()
    {}
};

} // namespace data_pool
} // namespace engine
} // namespace cpt
