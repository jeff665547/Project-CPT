#pragma once
#include <map>
#include <string>
#include <opencv2/opencv.hpp>
#include <CPT/format/json.hpp>
#include <CPT/spec/denali.hpp>
#include <CPT/improc/util.hpp>
#include <CPT/utility/language.hpp>
#include <CPT/spec/clariom_s.hpp>
#include <CPT/spec/zion.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
namespace bpt_ = boost::property_tree;
class DebugArgs
{
    // this class wont handle any image transform operation, all coordinate is define by spec
  public:
    DebugArgs() = default;
    template<class JSON>
    DebugArgs( JSON&& json )
    {
        set_chip_spec( FWD( json ) );
        set_debug_info ( std::forward<JSON>( json ) ) ;
    }
    // auto set_debug_flag( bool flag ) { debug_ = flag; }

    auto& get_region_by_id( uint16_t x, uint16_t y)
    {
        return p_region_list_->get_ext_value(x, y);
    }
    auto& get_region_by_org( const cv::Point& p )
    {
        auto id = p_region_list_->org2id( p.x, p.y );
        return get_region_by_id( id.first, id.second );
    }
    auto& get_img_info_by_id ( uint16_t x, uint16_t y )
    {
        return p_region_list_->get_img_info( x, y );
    }
    auto& get_img_info_by_org( const cv::Point& p ) 
    { 
        return get_img_info_by_id( p.x, p.y ); 
    }
    bool is_debug_on(const cv::Point& p )
    {
        auto id = p_region_list_->org2id( p.x, p.y );
        return debug_ &&  
            ( p_region_list_->get_ext_value( 
                  id.first
                , id.second 
            ).size() > 0 );
    }
    void imwrite( const std::string& filename, const cv::Mat& mat )
    {
        cpt::improc::imwrite( debug_dir_ / filename, mat );
    }
    
  private:
    template<class JSON>
    auto set_debug_info( JSON&& json )
    {
        if ( json.exists ( "debug" ) )
        {
            auto&& jdbg = json.get_child("debug");
            for ( auto&& v : jdbg.get_list("region") )
            {
                auto&& jv = cpt::format::make_json( v.second );
                auto x = jv.template get<int16_t>("x");
                auto y = jv.template get<int16_t>("y");
                auto w = jv.template get<int16_t>("w");
                auto h = jv.template get<int16_t>("h");
                for ( auto&& img : p_region_list_->get_map() )
                {
                    auto&& img_info = img.second.first;
                    if ( img_info.overlap(
                        x, y, w, h
                    ))
                    {
                        img.second.second.emplace_back( 
                            cv::Rect(
                                  x - img_info.org_pos.first
                                , y - img_info.org_pos.second
                                , w
                                , h
                            )
                        );
                    } 
                    
                }
            }
            debug_dir_ = jdbg.template get<std::string>("dir");
            boost::filesystem::create_directories( debug_dir_ );
            debug_ = true;
        }
        else { }
    }
    template<class JSON>
    auto set_chip_spec( JSON&& json )
    {
        auto spec = json.template get<std::string>( "spec.name" );
        if ( spec == "denali"  )
            p_region_list_.reset( new cpt::spec::DenaliDoc< std::vector< cv::Rect > > () );
        else if ( spec == "clariom_s" )
            p_region_list_.reset( new cpt::spec::ClariomSDoc< std::vector< cv::Rect > > () );
        else if ( spec == "zion" )
            p_region_list_.reset( new cpt::spec::ZionDoc< std::vector< cv::Rect > > () );
        else throw std::runtime_error( "unknown spec: " + spec );
    }
    bool debug_ { false };
    std::unique_ptr<cpt::spec::Doc< std::vector< cv::Rect > >> p_region_list_;
    boost::filesystem::path debug_dir_;
    
};

}}}
