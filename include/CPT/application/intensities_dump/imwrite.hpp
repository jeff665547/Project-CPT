#pragma once
#include <CPT/improc/src_r_o_i_infer.hpp>
#include <CPT/format/json.hpp>
#include <boost/filesystem.hpp>
#include <CPT/application/intensities_dump/spec.hpp>
#include <regex>
#include <CPT/application/intensities_dump/utility.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/range.hpp>
#include <CPT/application/intensities_dump/debug_args.hpp>
// #include <CPT/spec/denali/image_spec.hpp>
#include <CPT/view.hpp>
#include <CPT/improc/util.hpp>
#include <CPT/improc/background_fix/sub_and_division01.hpp>
#include <Nucleona/parallel/thread_pool.hpp>
namespace cpt { namespace application {
namespace intensities_dump {

struct ImWrite
{
    using This = ImWrite;
    struct Parameters
    {
        std::string fname_pos_pat;
        std::string spec_name;
        bool enable;
        bool dry;
        template<class BUFFER, class JSON>
        static void run ( BUFFER&& buf, JSON&& json )
        {
            auto&& opts = json.get_child("imwrite");
            buf.This::Parameters::config( opts );
        }
        template<class JSON>
        void config ( JSON&& opts )
        {
            enable = opts
                .template get_optional<decltype(enable)>("enable")
                .value_or(false);
            dry = opts
                .template get_optional<decltype(dry)>("verbose.dry")
                .value_or(false);
        }
    };
    template<class BUFFER>
    static auto img_debug(BUFFER&& buf, bool is_prerun = false)
    {
        cpt::application::intensities_dump::DebugArgs& debug_args 
            = *(buf.p_debug_args);
        if ( debug_args.is_debug_on( buf.abs_grid_probe_point ) )
        {
            auto mat_view( 
                cpt::view::make_matrix_transform_view(
                      cpt::view::make_row_major_view( 
                          buf.tiles
                        , buf.gl_x.size() - 1, buf.gl_y.size() - 1
                      )
                    , buf.origin_position
                    , buf.x_axis_direction
                )
            );
            auto&& region = debug_args.get_region_by_org( 
                buf.abs_grid_probe_point
            );
            // auto img_cell = cpt::improc::gray_log( buf.src.clone(), is_prerun ? 0.04 : 0.0 );
            auto fname = boost::filesystem::path(buf.img_path).stem();
            auto img_cell = cpt::improc::gray_log( buf.src.clone());
            debug_args.imwrite( 
                  fname.string() + "_graylog.tif"
                , img_cell
            );
            auto img_line = img_cell.clone();
            for ( decltype( buf.gl_y.size() )  y = 0; y < buf.gl_y.size() - 1; y ++ )
            {
                for ( decltype( buf.gl_x.size() ) x = 0; x < buf.gl_x.size() - 1; x ++ )
                {
                    auto& tile = mat_view.at( y, x );
                    
                    cv::rectangle( 
                          img_cell
                        , { tile.x, tile.y }
                        , { tile.x + tile.width, tile.y + tile.height }
                        , cv::Scalar(30000)
                    );
                        
                }
            }
            for ( auto&& yl : buf.gl_y )
            {
                cv::line( 
                      img_line
                    , { (int)buf.gl_x.front(), (int)yl }
                    , { (int)buf.gl_x.back() , (int)yl }
                    , cv::Scalar(30000)
                );
            }
            for ( auto&& xl : buf.gl_x )
            {
                cv::line( 
                      img_line
                    , { (int)xl, (int)buf.gl_y.front() }
                    , { (int)xl, (int)buf.gl_y.back()  }
                    , cv::Scalar(30000)
                );
            }
            if ( is_prerun )
            {
                debug_args.imwrite( 
                      fname.string() + "_shrink.tif"
                    , img_cell
                );
                debug_args.imwrite( 
                      fname.string() + "_line.tif"
                    , img_line
                );
            }
            else
            {
                for ( typename std::decay_t<decltype(region)>::size_type i (0); i < region.size(); i ++ )
                {
                    auto& r = region.at(i);
                    auto region_img = img_cell.clone();
                    for ( int16_t y = r.y; y < r.y + r.height; y ++ )
                    {
                        for ( int16_t x = r.x; x < r.x + r.width; x ++ )
                        {
                            auto& tile = mat_view.at( y, x );
                            
                            cv::rectangle( 
                                  region_img
                                , { tile.x, tile.y }
                                , { tile.x + tile.width, tile.y + tile.height }
                                , cv::Scalar(65535)
                            );
                        }
                    }
                    debug_args.imwrite( 
                          "region_" + std::to_string( i ) + ".tif"
                        , region_img
                    );
                }
            }

        }
    }
    template<class BUFS>
    static void run_bufs( BUFS&& bufs )
    {
        auto tp ( nucleona::parallel::make_thread_pool(8));
        std::vector<float> min (bufs.size(), std::numeric_limits<float>::max());
        std::vector<float> max (bufs.size(), std::numeric_limits<float>::min());
        std::vector< std::pair< int, int > > first_floor_loc( bufs.size(), {-1,-1} );
        std::vector<decltype(bufs.at(0).src)> log_srcs( bufs.size() );
        for ( int i = 0; i < bufs.size(); i ++ )
        {
            if ( !bufs.at(i).Parameters::enable ) continue;
            auto* const p_buf = &bufs.at(i);
            auto* const p_lmin = &min.at(i);
            auto* const p_lmax = &max.at(i);
            auto* const p_ffl = &first_floor_loc.at(i);
            auto* const p_log_src = &log_srcs.at(i);
            tp.job_post( [
                  &p_buf
                , p_lmin
                , p_lmax
                , p_ffl
                , p_log_src
            ]()
            {
                auto& buf = *p_buf;
                auto& log_src = *p_log_src;
                log_src = buf.src.clone();
                cv::log( log_src, log_src );
                for ( int i = 0; i < log_src.rows; i ++ )
                {
                    for ( int j = 0; j < log_src.cols; j ++ )
                    {
                        float& v = log_src.template at<float>( i, j );
                        if ( v <= cpt::improc::background_fix::SegmentMean::floor ) 
                        {
                            if ( p_ffl->first == -1 )
                            {
                                p_ffl->first = i;
                                p_ffl->second = j;
                            }
                            continue;
                        }
                        if ( v > *p_lmax ) *p_lmax = v;
                        if ( v < *p_lmin ) *p_lmin = v;
                    }
                }
            });
        }
        tp.flush();

        decltype(min.size()) min_i = 0;
        decltype(max.size()) max_i = 0;
        for ( decltype(min.size()) i = 0; i < min.size(); i ++ ) 
        {
            if ( min.at(i) < min.at(min_i) ) min_i = i;
            if ( max.at(i) > max.at(max_i) ) max_i = i;
        }
        const float gmax = max.at(max_i);
        const float gmin = min.at(min_i);

        // ========> global min max and first floor location handle done....
        // now we normalize all matrix to global min max 
        for ( int i = 0; i < log_srcs.size(); i ++ )
        {
            if ( !bufs.at(i).Parameters::enable ) continue;
            auto* const p_buf = &bufs.at(i);
            auto img_name_pos_parser( make_impos_parser( 
                  p_buf->Parameters::fname_pos_pat
                , p_buf->Parameters::spec_name
            ));
            auto fname = boost::filesystem::path(p_buf->img_path).filename();
            auto tfname = fname.string();
            boost::filesystem::path raw_img;
            boost::filesystem::path log_img;
            auto pp ( img_name_pos_parser->parse( tfname ) );
            raw_img = ( std::to_string(pp.second) + "_" + std::to_string(pp.first) ) + ".png" ;
            log_img = ( std::to_string(pp.second) + "_" + std::to_string(pp.first) ) + "_log.png" ;
            auto raw_img_opath = p_buf->output_dir / raw_img;
            auto log_img_opath = p_buf->output_dir / log_img;

            auto* const p_log_src = &log_srcs.at(i);
            auto* const p_ffl = &first_floor_loc.at(i);
            tp.job_post([
                  p_log_src
                , p_ffl
                , gmin
                , gmax
                , log_img_opath
                , raw_img_opath
                , p_buf
            ]()
            {
                auto& log_src = *p_log_src;
                for ( int i = 0; i < log_src.rows; i ++ )
                {
                    for ( int j = 0; j < log_src.cols; j ++ )
                    {
                        // re-assign every value small than floor to min
                        float& v = log_src.template at<float>( i, j );
                        if ( v <= cpt::improc::background_fix::SegmentMean::floor )
                        {
                            v = gmin;
                        }
                    }
                }
                // assign the max to first floor location, we will assgin 0 back to this location, 
                // to do this step is because we have to let cv minmax work.....
                auto no_log = p_buf->src.clone();
                cv::normalize( no_log, no_log, 0, 65535, cv::NORM_MINMAX, CV_16U );

                log_src.template at<float>( p_ffl->first, p_ffl->second ) = gmax;
                cv::normalize( log_src, log_src, 0, 65535, cv::NORM_MINMAX, CV_16U );
                log_src.template at<float>( p_ffl->first, p_ffl->second ) = 0;
                cv::imwrite( raw_img_opath.string(), no_log );
                cv::imwrite( log_img_opath.string(), log_src );
            });
        }
        tp.flush();
    }

    template<class BUFFER>
    static void run ( BUFFER&& buf )
    {}

    // template<class BUFFER>
    // static void run ( BUFFER&& buf )
    // {
    //     img_debug( buf, true );
    //     if ( buf.Parameters::enable )
    //     {
    //         boost::filesystem::path raw_img;
    //         boost::filesystem::path log_img;
    //         auto img_name_pos_parser( make_impos_parser( 
    //               buf.Parameters::fname_pos_pat
    //             , buf.Parameters::spec_name
    //         ));

    //         auto fname = boost::filesystem::path(buf.img_path).filename();
    //         auto tfname = fname.string();
    //         auto pp ( img_name_pos_parser->parse( tfname ) );
    //         raw_img = ( std::to_string(pp.second) + "_" + std::to_string(pp.first) ) + ".png" ;
    //         log_img = ( std::to_string(pp.second) + "_" + std::to_string(pp.first) ) + "_log.png" ;
    //         auto raw_img_opath = buf.output_dir / raw_img;
    //         auto log_img_opath = buf.output_dir / log_img;

    //         boost::filesystem::path q;
    //         if ( !buf.Parameters::dry )
    //         {
    //             if ( !boost::filesystem::exists( buf.output_dir ) )
    //             {
    //                 boost::filesystem::create_directories( buf.output_dir );
    //             }
    //             cpt::msg << log_img_opath << " : image save" << std::endl;
    //             
    //             cv::Mat log_res = cpt::improc::gray_log( buf.src );;
    //             cv::normalize( log_res, log_res, 0, 65535, cv::NORM_MINMAX, CV_16U );
    //             cv::imwrite( log_img_opath.string(), log_res );

    //             cpt::msg << raw_img_opath << " : image save" << std::endl;
    //             buf.src.convertTo(buf.src, CV_16U);
    //             cv::imwrite( raw_img_opath.string(), buf.src ); 

    //         }
    //         img_debug( buf );
    //     }
    // }

};

}}}
