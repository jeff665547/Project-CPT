#pragma once
#include <utility>
#include <cstdint>
namespace cpt { namespace spec { 

struct ImageInfo
{
    std::pair<uint16_t, uint16_t> length;
    std::pair<uint16_t, uint16_t> org_pos;
    std::pair<uint16_t, uint16_t> id;
    bool f_overlap ( uint16_t f, uint16_t fl ) const 
    {
        auto img_fend = org_pos.first + length.first;
        auto fend = f + fl;
        if ( f >= img_fend || fend <= org_pos.first )
        {
            return false;
        }
        else return true;

    }
    bool s_overlap ( uint16_t s, uint16_t sl ) const 
    {
        auto img_send = org_pos.second + length.second;
        auto send = s + sl;
        if ( s >= img_send || send <= org_pos.second )
        {
            return false;
        }
        else return true;

    }
    bool overlap ( uint16_t f, uint16_t s, uint16_t fl, uint16_t sl ) const 
    {
        return f_overlap( f, fl ) && s_overlap( s, sl ) ;
    }
};
struct ImageInfoLess
{
    using ImgInfoId = decltype(ImageInfo::id);
    auto operator()(  const ImgInfoId& a, const ImgInfoId& b )
    {
        if ( a.first == b.first )
            return a.second < b.second;
        else return a.first < b.first;
    }
};

}}
