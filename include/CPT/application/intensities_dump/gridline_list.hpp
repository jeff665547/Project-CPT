#pragma once
#include <vector>
#include <utility>
#include <cstdint>
#include <CPT/format/json.hpp>
#include <regex>
#include <CPT/spec/denali.hpp>
#include <CPT/application/intensities_dump/utility.hpp>
#include <CPT/utility/unique_ptr.hpp>
#include <CPT/spec/clariom_s.hpp>
#include <CPT/application/intensities_dump/utility.hpp>
#include <CPT/spec/zion/image_spec.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
using GLS = std::vector<uint32_t>;
struct ImgGridline
{
    ImgGridline( GLS& gl_x, GLS& gl_y )
    : p_gl_x_ ( &gl_x )
    , p_gl_y_ ( &gl_y )
    {
        
    }
    auto& gl_x()
    {
        return *p_gl_x_;
    }
    auto& gl_y()
    {
        return *p_gl_y_;
    }
  private:
    std::vector<uint32_t>* p_gl_x_, *p_gl_y_;
};
// struct ImagePositionParser
// {
//     virtual std::pair<int, int> parse( const std::string& name ) const = 0;
// };
// struct ImNamePosMatcher : public ImagePositionParser
// {
//     ImNamePosMatcher( const std::string& fname_pat )
//     : img_pos_pattern_ ( fname_pat )
//     , raw_pat_ ( fname_pat )
//     {}
// 
//     virtual std::pair<int, int> parse ( const std::string& fname ) const override
//     {
//         // FOV(\d+)_(\d+)
//         std::smatch m;
//         if ( std::regex_search ( fname, m, img_pos_pattern_ ) )
//         {
//             // return pos_str2pair( (m.begin() + 1 )->str() );
//             return std::make_pair( std::stoi(m[1]), std::stoi(m[2]) );
//         }
//         std::stringstream ss;
//         ss << "pattern not found : " << std::endl;
//         ss << "string : "   << fname    << std::endl;
//         ss << "pattern : "  << raw_pat_ << std::endl;
//         throw std::logic_error ( ss.str() ); 
//     }
//   private:
//     std::regex img_pos_pattern_;
//     std::string raw_pat_;
// };
// struct ZeroMatcher : public ImagePositionParser
// {
//     ZeroMatcher(){}
//     virtual std::pair<int, int> parse ( const std::string& fname ) const override
//     {
//         return { 0, 0 };
//     }
// };
struct GridlineList
{
    GridlineList( const std::string& fname_pat, const std::string& spec = "denali" )
    : data_     ( cpt::format::make_json()                  )
    , parser_   ( make_impos_parser     ( fname_pat, spec ) )
    , img_spec_ ( make_image_spec       ( spec )            )
    {}
    auto pair2str ( const std::pair<int, int>& p )
    {
        return std::to_string( p.second ) + "_" + std::to_string( p.first );
    }
    auto add_img_gl ( const std::string& fname, GLS& gl_x, GLS& gl_y )
    {
        // auto pos = matcher_( fname );
        auto pos = parser_->parse ( fname );
        auto&& fnode = data_.create_child( pair2str(pos) );

        auto&& jorg_pos = fnode.create_list("org_pos");
        auto&& org_pos = img_spec_.at( pos ).org_pos;
        jorg_pos.push_back( org_pos.second );
        jorg_pos.push_back( org_pos.first );

        auto&& jprobe_num = fnode.create_list("probe_num");
        auto&& probe_num = img_spec_.at( pos ).length;
        jprobe_num.push_back( probe_num.second );
        jprobe_num.push_back( probe_num.first );

        auto&& xgl = fnode.create_list ( "gl_x" );
        auto&& ygl = fnode.create_list ( "gl_y" );
        for ( auto&& l : gl_x )
            xgl.push_back( l );
        for ( auto&& l : gl_y )
            ygl.push_back( l );
    }
    template<class OS>
    auto dump ( OS&& os )
    {
        data_.dump ( os, false );
    }
  private:
    cpt::spec::ImageSpecMap make_image_spec( const std::string& spec )
    {
        if ( spec == "denali" ) return cpt::spec::denali::image_spec_map();
        else if ( spec == "clariom_s" ) return cpt::spec::clariom_s::image_spec_map();
        else if ( spec == "zion" ) return cpt::spec::zion::image_spec_map();
        throw std::runtime_error( "unknown spec: " + spec );
    }
    // std::unique_ptr<
    //    ImagePositionParser
    // > make_impos_parser( const std::string& fname_pat, const std::string& spec )
    // {
    //     if( spec=="denali" || fname_pat != "" ) 
    //         return cpt::utility::make_unique( new ImNamePosMatcher(fname_pat ) );
    //     else 
    //         return cpt::utility::make_unique( new ZeroMatcher( ) );
    // }
    cpt::format::Json< boost::property_tree::ptree > data_;
    std::unique_ptr<ImagePositionParser> parser_;
    cpt::spec::ImageSpecMap img_spec_;
};
// struct TsvGridlineList
// {
//     TsvGridlineList( const std::string& fname_pat )
//     : matcher_  ( fname_pat )
//     , img_spec_ ( cpt::spec::denali::image_spec_map() )
//     {}
//     auto add_img_gl ( const std::string& fname, GLS& gl_x, GLS& gl_y )
//     {
//         auto pos = pos_str2pair( matcher_( fname ) );
//         data_.emplace_back(
//             Entry{ 
//                   pos
//                 , img_spec_.at( pos ).org_pos
//                 , img_spec_.at( pos ).length
//                 , std::make_pair( gl_x, gl_y ) 
//                 // TODO assume gl_x is from long edge
//             }
//         );
//     }
//     template<class OS>
//     auto dump ( OS&& os )
//     {
//         for ( auto&& d : data_ )
//         {
//             d.dump( os );
//         }
//     }
//   private:
//     struct Entry
//     {
//         std::pair<int, int> img_pos;
//         std::pair<uint16_t, uint16_t> img_org_pos;
//         std::pair<uint16_t, uint16_t> img_probe_num;
//         std::pair<GLS, GLS> gl;
//         template<class VEC, class OS>
//         auto& dump_vec( VEC&& vec, OS& os )
//         {
//             os << vec.at(0);
//             for ( typename std::decay_t<decltype(vec)>::size_type i (1); 
//                 i < vec.size(); 
//                 i ++ 
//             )
//             {
//                 os << ',' << vec.at(i);
//             }
//             return os;
//         }
//         template<class OS>
//         auto& dump( OS&& os )
//         {
//             os  << img_pos.second << "_" << img_pos.first
//                 << '\t' << img_org_pos.second << ',' << img_org_pos.first
//                 << '\t' << img_probe_num.second << ',' << img_probe_num.first
//                 << '\t'
//             ;
//             dump_vec( gl.second, os ) << '\t';
//             dump_vec( gl.first, os ) << '\n';
//             return os;
//         }
//     };
//     ImNamePosMatcher matcher_;
//     std::vector< Entry > data_;
//     std::result_of_t<decltype(cpt::spec::denali::image_spec_map)&()> img_spec_;
// };

}}}
