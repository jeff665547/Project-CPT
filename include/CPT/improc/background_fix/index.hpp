#pragma once
#include <map>
#include <CPT/spec/image_spec.hpp>
#include <CPT/view/row_major_view.hpp>
namespace cpt{ namespace improc{ namespace background_fix{
using IndexProto = std::map< 
      cpt::spec::ImageInfoId
    , cpt::view::RowMajorView<std::vector<float>>
    , cpt::spec::ImageInfoLess
>;

struct Index : public IndexProto
{
    using Base = IndexProto;
    Index( 
          uint16_t image_x_nums
        , uint16_t image_y_nums
        , uint16_t segment_width
        , uint16_t segment_height
    )
    : image_x_nums_     ( image_x_nums      )
    , image_y_nums_     ( image_y_nums      )
    , segment_width_    ( segment_width     )
    , segment_height_   ( segment_height    )
    {}

    decltype(auto) at( 
          typename cpt::spec::ImageInfoId::first_type x
        , typename cpt::spec::ImageInfoId::second_type y
    )
    {
        return IndexProto::at( cpt::spec::ImageInfoId{ x, y } );
    }
  private:
    uint16_t image_x_nums_       ;
    uint16_t image_y_nums_       ;
    uint16_t segment_width_      ;
    uint16_t segment_height_     ;
};

}}}
