#pragma once
#include <vector>
#include <utility>
#include <CPT/spec/image_info.hpp>
#include <map>
#include <CPT/spec/image_spec.hpp>
namespace cpt { namespace spec { namespace denali{
auto image_spec_map_base(uint16_t fe, uint16_t se)
{
    return equal_size_image_spec_map( fe, se, 126, 126, 10, 10 );
}
auto image_spec_map() // by file
{
    return image_spec_map_base( 17, 3 );
}
auto image_spec_map_by_doc() 
{
    return image_spec_map_base( 3, 17 );
}

}}}
