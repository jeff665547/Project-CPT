#pragma once
#include <boost/variant.hpp>
namespace cpt {
namespace format {

typedef boost::variant<
      int8_t
    , int16_t
    , int32_t
    , uint8_t
    , uint16_t
    , uint32_t
    , double
    , float
    , char
    , std::string
> GeneralBasicVariant;

}}
