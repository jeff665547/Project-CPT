#pragma once
#include <string>
#include <CPT/application/ui_img_index/indexer.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <CPT/utility.hpp>
#include <CPT/format/json.hpp>

namespace cpt { namespace application {
namespace ui_img_index {
namespace _ {
// struct ImageIndex
// {
//     struct Node
//     { 
//         uint16_t pos; 
//         bool start;
//         const ImageInfo* image_info; 
//     };
//     std::vector<Node> first;
//     std::vector<Node> second;
//     ImageIndex ( const std::vector< ImageInfo >& image_spec )
//     {
//         for ( auto&& is : image_spec )
//         {
//             first.emplace_back( 
//                 is.org_pos.first
//                 , true
//                 , &is
//             );
//             first.emplace_back(
//                 is.org_pos.first + is.length.first
//                 , false
//                 , &is
//             );
//             second.emplace_back(
//                 is.org_pos.second
//                 , true
//                 , &is
//             );
//             second.emplace_back(
//                 is.org_pos.second + is.length.second
//                 , false
//                 , &is
//             );
//         }
//         std::sort( first.begin(), first.end(), []( auto& a, auto& b )
//         {
//             if ( a.pos == b.pos )
//             {
//                 if ( a.start == b.start )
//                 {
//                     return (int&)a.image_info < (int&)b.image_info;
//                 }
//                 else return a.start < b.start ;
//             }
//             else return a.pos < b.pos;
// 
//         });
//     }
//     auto interal ( uint16_t f, uint16_t s, uint16_t fl, uint16_t sl )
//     {
//         
// 
//     }
// };
}
namespace bf_ = boost::filesystem;
struct ImageInfo
{
    std::pair<uint16_t, uint16_t> length;
    std::pair<uint16_t, uint16_t> org_pos;
    std::pair<uint16_t, uint16_t> id;
    bool overlap ( uint16_t f, uint16_t s, uint16_t fl, uint16_t sl ) const 
    {
        auto foffset = (int)f - org_pos.first;
        if ( foffset < 0 || foffset > length.first )
        {
            return false;
        }
        auto soffset = (int)s - org_pos.first;
        if ( soffset < 0 || soffset > length.second )
        {
            return false;
        }
        return true;
    }
};
struct Grid
{
    auto get_grid_size( const std::vector<ImageInfo>& image_infos )
    {
        std::pair<uint16_t, uint16_t> max { 0, 0 };
        for ( auto&& ii : image_infos )
        {
            auto tmp_first = ii.org_pos.first + ii.length.first;
            auto tmp_second = ii.org_pos.second + ii.length.second;
            if ( tmp_first > max.first )
                max.first = tmp_first;
            if ( tmp_second > max.second )
                max.second = tmp_second;
        }
        return max;
    }
    std::vector<std::vector<const ImageInfo*>> content;
    std::pair<uint16_t, uint16_t> length;
    Grid(const std::vector<ImageInfo>& image_infos)
    {
        length = get_grid_size( image_infos );
        content.resize( length.first * length.second );
        for ( auto&& ii : image_infos ) // image number dependent ( constent )
        {
            for ( auto f(ii.org_pos.first); f < ii.org_pos.first + ii.length.first; f ++ ) // image size dependent
            {
                for ( auto s(ii.org_pos.second); s < ii.org_pos.second + ii.length.second; s ++ ) // image size dependent
                {
                    content.at( length.first * s + f ).push_back( &ii ); 
                }
            }

        }
    }
    auto operator()( uint16_t grid_x, uint16_t grid_y )
    {
        return content.at(
            length.first * grid_x + grid_y 
        ); // xyfs_conversion
    }
};
struct StupidImageIndex
{
    StupidImageIndex ( const std::vector< ImageInfo >& image_spec )
    : image_spec_ (image_spec )
    {}
    auto operator()( uint16_t f, uint16_t s, uint16_t fl, uint16_t sl )
    {
        std::vector<const ImageInfo*> res;
        for ( auto&& is : image_spec_ )
        {
            if ( is.overlap( f, s, fl, sl ) )
                res.push_back( &is );
        }
        return res;
    }
  private: 
    const std::vector<ImageInfo>& image_spec_;
};

auto denali_image_spec()
{
    std::vector<ImageInfo> image_infos;
    uint16_t org_pos_first = 0;
    for ( uint16_t first(0); first < 17; first ++ )
    {
        uint16_t org_pos_second = 0;
        for ( uint16_t second(0); second < 3; second ++ )
        {
            image_infos.push_back(
                ImageInfo {
                      { 126, 126 }
                    , { org_pos_first, org_pos_second }
                    , { first, second }
                }
            );
            org_pos_second += 116;
        }
        org_pos_first += 116;
    }
    return image_infos;
}
struct PixelBasedIndexer : public Indexer
{
    virtual void operator() (
        const Parameters& param
    ) override
    {
        bf_::path dir ( param.input_dir );
        auto&& jgridline = cpt::format::read_json( (dir / "gridline.json")
                .string() );
        auto&& jconfig   = cpt::format::read_json( param.config );
        StupidImageIndex image_spec ( denali_image_spec() );

        auto&& tile_dict = cpt::format::make_json();
        for ( uint16_t y(0); 
            y < jconfig.get<uint16_t>("num_rows") / param.tile_height;
            y ++ )
            for ( uint16_t x(0); 
                x < jconfig.get<uint16_t>("num_cols") / param.tile_width; 
                x ++ )
            {
                auto tile_grid_op_x = x * param.tile_width;
                auto tile_grid_op_y = y * param.tile_height;
                auto id = std::to_string( tile_grid_op_y ) + "_" + std::to_string( tile_grid_op_x );
                auto&& overlap_img_info_list = image_spec( y, x, param.tile_height, param.tile_width ); // xy2fs
                auto&& tile_info = tile_dict.create_child( id );
                auto&& ovimgs = tile_info.create_list("overlap_imgs");
                for ( auto&& oii : overlap_img_info_list )
                {
                    auto&& img = ovimgs.create_child();
                    auto&& rel_pos = img.create_list("rel_pos");
                    auto roi_x = tile_grid_op_x - oii->org_pos.second;  // xy2fs
                    auto roi_y = tile_grid_op_x - oii->org_pos.first;   // xy2fs
                    rel_pos.push_back( roi_y );
                    rel_pos.push_back( roi_x );
                }
            }
    }
};


}}}
