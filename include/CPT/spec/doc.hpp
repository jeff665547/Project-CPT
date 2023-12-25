#pragma once
#include <cstdint>
#include <utility>
#include <CPT/spec/image_info.hpp>
#include <map>
#include <CPT/spec/image_spec.hpp>

namespace cpt { namespace spec { 

template<class T>
struct Doc
{
    using ImageIndex = std::map<
        ImageInfoId
        , std::pair<
              ImageSpecMap::mapped_type
            , T
        >
        , ImageSpecMap::key_compare
    >;
    virtual ImageInfoId org2id( uint16_t x, uint16_t y ) const = 0;
    virtual T&          get_ext_value( uint16_t x, uint16_t y ) = 0;
    virtual ImageIndex& get_map() = 0;

    virtual typename ImageSpecMap::mapped_type& 
                        get_img_info( uint16_t x, uint16_t y ) = 0;
};


}}
