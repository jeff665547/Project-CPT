#pragma once
#include <vector>
#include <utility>
#include <CPT/spec/image_info.hpp>
#include <map>
namespace cpt { namespace spec {

using ImageInfoId = decltype(ImageInfo::id);
using ImageSpecMap = std::map<
      ImageInfoId
    , ImageInfo
    , ImageInfoLess
>;
auto equal_size_image_spec_map(
      uint16_t fe
    , uint16_t se
    , uint16_t img_f
    , uint16_t img_s
    , uint16_t marker_f
    , uint16_t marker_s
)
{
    using ImgInfoId = decltype(ImageInfo::id);
    ImageInfoLess comp;
    std::map< 
          ImgInfoId
        , ImageInfo
        , decltype(comp) 
    > res( comp );
    uint16_t org_pos_first = 0;
    for ( uint16_t first(0); first < fe; first ++ )
    {
        uint16_t org_pos_second = 0;
        for ( uint16_t second(0); second < se; second ++ )
        {
            res.emplace(
                  ImgInfoId { first, second }
                , ImageInfo {
                      { img_f, img_s }
                    , { org_pos_first, org_pos_second }
                    , { first, second }
                }
            );
            org_pos_second += ( img_s - marker_s );
        }
        org_pos_first += ( img_f - marker_f );
    }
    return res;
}

}}
