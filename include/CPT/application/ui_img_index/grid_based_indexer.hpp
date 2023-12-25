#pragma once
#include <string>
#include <CPT/application/ui_img_index/indexer.hpp>
#include <CPT/application/ui_img_index/option.hpp>
#include <CPT/format/json.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <CPT/spec/image_info.hpp>
#include <CPT/spec/denali.hpp>
#include <CPT/improc/util.hpp>
#include <CPT/application/ui_img_index/img_manager.hpp>
#include <CPT/application/ui_img_index/gridline_manager.hpp>
#include <cmath>
#include <CPT/logger.hpp>
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
using ImageInfo = cpt::spec::ImageInfo;
using ImageSpecMap = decltype(cpt::spec::denali::image_spec_map());
namespace bf_ = boost::filesystem;
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
    StupidImageIndex ( const ImageSpecMap& image_spec )
    : image_spec_ (image_spec )
    {}
    auto operator()( uint16_t f, uint16_t s, uint16_t fl, uint16_t sl )
    {
        std::vector<const ImageInfo*> res;
        for ( auto&& is : image_spec_ )
        {
            if ( is.second.overlap( f, s, fl, sl ) )
                res.push_back( &is.second );
        }
        return res;
    }
  private: 
    ImageSpecMap image_spec_;
};
struct GridBasedIndexer : public Indexer
{
    template<class F, class S>
    auto pair2str( const std::pair<F,S>& p )
    {
        return std::to_string( p.second ) + "_" + std::to_string( p.first );
    }
    auto img_write( 
          const std::string& tile_id_prefix
        , const std::string& img_id
        , const std::map<std::string, std::string>& pos_map
        , const std::pair<std::string, cv::Mat>& tile_img
    )
    {
        assert( tile_img.second.cols > 0 );
        assert( tile_img.second.rows > 0 );
        cim_::imwrite( 
              tile_id_prefix 
                + "_"  + ( img_id + pos_map.at( tile_img.first ) )
            , tile_img.second
        );
    }
    auto all_roi( 
          const std::map<std::string, cv::Mat>& imgs
        , int16_t roi_x
        , int16_t roi_y
        , int16_t roi_w
        , int16_t roi_h
        , const std::vector<uint32_t>& gl_x
        , const std::vector<uint32_t>& gl_y
    )
    {
        std::decay_t<decltype(imgs)> res;
        for ( auto&& type_img : imgs )
        {
            auto mat_x = gl_x.at( std::max( roi_x, (int16_t)0 ) );
            auto mat_y = gl_y.at( std::max( roi_y, (int16_t)0 ) );
            auto mat_w = gl_x.at( std::min( (int16_t)(roi_x + roi_w), (int16_t)(gl_x.size() - 1)) ) - mat_x;
            auto mat_h = gl_y.at( std::min( (int16_t)(roi_y + roi_h), (int16_t)(gl_y.size() - 1)) ) - mat_y;
            assert( mat_w > 0 );
            assert( mat_h > 0 );

            cv::Rect roi( mat_x, mat_y, mat_w, mat_h );
            res.emplace(type_img.first, type_img.second(roi));
        }
        return res;
        
    }
    uint16_t div(float o1, float o2 )
    {
        return std::ceil(o1 / o2);

    }
    virtual void operator() (
        const Parameters& param
    ) override
    {
        bf_::path dir ( param.input_dir );
        auto&& jgridline = cpt::format::read_json( 
            (dir / "gridline.json").string() );
        StupidImageIndex image_spec ( cpt::spec::denali::image_spec_map() );
        std::map<std::string, std::string> pos_map  
        { 
            { "log", "_log.png" } 
            , { "raw", ".png" }
        }
        ;
        ImgManager img_manager( 
              dir
            , pos_map
        );
        bf_::path tile_img_dir = param.tile_img_prefix;
        GridlineManager gl_manager( (dir / "gridline.json").string() ); 

        auto&& tile_dict = cpt::format::make_json();
        for ( uint16_t y(0); 
            y < div( cpt::spec::denali::num_rows, param.tile_height);
            y ++ )
        {
            for ( uint16_t x(0); 
                x < div( cpt::spec::denali::num_cols, param.tile_width) ; 
                x ++ )
            {
                // x short y long
                auto tile_grid_op_x = x * param.tile_width;
                auto tile_grid_op_y = y * param.tile_height;
                auto id = std::to_string( x ) + "_" + std::to_string( y ); // tile_id
                auto&& overlap_img_info_list = image_spec( 
                      tile_grid_op_y
                    , tile_grid_op_x
                    , param.tile_width
                    , param.tile_height 
                ); // xy2fs tile vs spec
                auto&& tile_info = tile_dict.create_child( id );
                auto&& ovimgs = tile_info.create_list("overlap_imgs");
                auto tile_id_prefix( tile_img_dir / id );
                bf_::create_directories( tile_img_dir );
                for ( auto&& oii : overlap_img_info_list )
                {
                    auto&& img = ovimgs.create_child();
                    auto&& img_id = pair2str ( oii->id ); // parent image id
                    img.add( "id", img_id );
                    auto&& rel_pos = img.create_list("rel_pos");

                    auto roi_x = tile_grid_op_x - oii->org_pos.second;  // xy2fs 
                    auto roi_y = tile_grid_op_y - oii->org_pos.first;   // xy2fs
                    rel_pos.push_back( roi_x );
                    rel_pos.push_back( roi_y );
                    auto&& img_data = img_manager( img_id );
                    auto gl_info ( gl_manager( img_id ) );
                    
                    for ( auto&& tile_img : all_roi( 
                          img_data
                        , roi_y
                        , roi_x
                        , param.tile_width
                        , param.tile_height
                        , gl_info.gl_x
                        , gl_info.gl_y
                     ) ) // 4 element at most
                    {
                        img_write( tile_id_prefix.string(), img_id, pos_map, tile_img );
                    }
                }
                cpt::msg << tile_id_prefix.string() << " processed" << std::endl;
            }
        }

        switch ( param.mode )
        {
            case Parameters::single:
            {
                tile_dict.dump( std::ofstream(param.output_str) );
                break;
            }
            case Parameters::multi:
            {
                boost::filesystem::path odir ( param.output_str );
                if ( !boost::filesystem::exists( odir ) ) boost::filesystem::create_directories( odir );
                for ( auto&& p : tile_dict.root )
                {
                    auto&& json = cpt::format::make_json(p.second);
                    json.dump( std::ofstream((odir / ( p.first + ".json" )).string()) );
                }
                break;
            }
        }
    }
};



}}}
