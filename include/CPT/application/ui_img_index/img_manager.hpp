#pragma once
#include <string>
#include <map>
#include <CPT/improc/util.hpp>
namespace cpt { namespace application {
namespace ui_img_index {
namespace cim_ = cpt::improc;
namespace bf_ = boost::filesystem;
struct ImgManager : public std::map<
      std::string
    , std::map<std::string, cv::Mat>
>
{
    ImgManager ( 
          const bf_::path& img_dir
        , const std::map<std::string, std::string>& pos_map 
    )
    : img_dir_( img_dir )
    , pos_map_( pos_map )
    {}
    auto read_img( const std::string& id )
    {
        std::map<std::string, cv::Mat> res;
        for ( auto&& p : pos_map_ )
        {
            res.emplace(p.first, cim_::imread( img_dir_ / ( id + p.second) ));
        }
        // res.emplace( "log", cim_::imread( img_dir_ / ( id + "_log.png" ) ) );
        // res.emplace( "raw", cim_::imread( img_dir_ / ( id + ".png" ) ) );
        return res;
    }
    auto& operator() ( const std::string& id )
    {
        auto itr( this->find( id ) );
        if ( itr != this->end() )
        {
            return itr->second;
        }
        else
        {
            // TODO maybe need to release resource here
            auto p ( this->emplace_hint( itr, id, read_img( id ) ));
            return p->second;
        }
    }
  private:
    bf_::path img_dir_;
    std::map<std::string, std::string> pos_map_;
};
}}}
