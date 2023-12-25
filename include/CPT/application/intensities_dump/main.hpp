#include <CPT/application/intensities_dump/static_config.hpp>
#include <CPT/logger.hpp>
#include <CPT/application/improc2/utils.hpp>
#include <CPT/application/improc2/improc.hpp>
#include <CPT/option_parser.hpp>
#include <CPT/application/intensities_dump/extract.hpp>
#include <CPT/application/intensities_dump/summarize.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <CPT/application/intensities_dump/pipe_control.hpp>
#include <CPT/application/intensities_dump/r_o_i_detection.hpp>
#include <CPT/application/intensities_dump/rotation_estimation.hpp>
#include <cmath>
#include <CPT/application/intensities_dump/brightness_fix.hpp>
#include <CPT/application/intensities_dump/src_r_o_i_infer.hpp>
#include <CPT/application/intensities_dump/gridline_list.hpp>
#include <CPT/application/intensities_dump/partial_grid.hpp>
#include <CPT/application/intensities_dump/stitched_grid.hpp>
#include <CPT/format/chip_sample.hpp>
#include <Nucleona/language.hpp>
#include <functional>
#include <CPT/format/converter/array2cel_converter.hpp>
#include <CPT/format/converter/array2cen_converter.hpp>
#include <CPT/format/converter/array2il_converter.hpp>
#include <CPT/application/intensities_dump/debug_args.hpp>
#include <string>
#include <CPT/application/intensities_dump/min_cv_auto_margin.hpp>
// #include <CPT/application/intensities_dump/segmentation.hpp>
#include <CPT/application/intensities_dump/report.hpp>
// #include <CPT/application/intensities_dump/middle_segmentation.hpp>
#include <CPT/application/intensities_dump/spec.hpp>
// #include <CPT/application/intensities_dump/background_fix/raw_img_sub_base01.hpp>
#include <CPT/application/intensities_dump/background_fix/probe_grid_sub_and_division_base01.hpp>
#include <CPT/application/intensities_dump/comp_adapt.hpp>
#include <CPT/application/intensities_dump/imwrite.hpp>
#include <CPT/application/intensities_dump/coordinate_fix.hpp>
#include <CPT/application/intensities_dump/background_fix/global_factor_process.hpp>
#include <CPT/application/intensities_dump/background_fix.hpp>

namespace cpt { namespace application {
namespace intensities_dump {
namespace im_   = cpt::application::improc2 ;
namespace cfcs_ = cpt::format::chip_sample  ;
namespace cfc_  = cpt::format::converter    ;

class OptionParser : public cpt::OptionParser
{
template<class T> friend class Main;
    std::string         config_         ;
    std::string         output_         ;
    std::string         image_args_     ;
    std::string         output_format_  ;
    bool                debug_          ;
  public:
    OptionParser(int argc, char const * argv[])
    {
        po::options_description desc( "Allowed options" );
        desc.add_options()
            ( "help,h"          , "show help message" )
            ( "config,c"        , po::value<std::string>(), "config"            )
            ( "output,o"        , po::value<std::string>(), "output"            )
            ( "image_args,i"    , po::value<std::string>(), "input images"      )
            ( "output_format,f" , po::value<std::string>()->default_value("il"), "output format (cel/cen/il)[il]" )
            ( "debug,d"         , po::value<bool>()->default_value(false)
              , "turn on debug mode [off]" )
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if(vm.count("help") || argc <= 1)
        {
            cpt::msg << desc << std::endl;
            exit(1);
        } 
        this->get_parameter( "config"       , config_           );
        this->get_parameter( "output"       , output_           );
        this->get_parameter( "image_args"   , image_args_       );
        this->get_parameter( "output_format", output_format_    );
        this->get_parameter( "debug"        , debug_            );
    }
};
struct ImgGrid
{
    using PC = PipeControl<
          BrightnessFix
        , RotationEstimation
        , im_::RotationCalibration
        , im_::Gridding
        // , MiddleSegmentation
        // , Segmentation
        , MinCVAutoMargin
        , ROIDetectionWithLayout
        , SrcROIInfer
        , BackgroundProcess
        // , background_fix::PartialProbeGridSubAndDivisionBase01
    >;
    using PC2 = PipeControl<
          background_fix::GlobalBGFactorProcess
        , im_::Summary<im_::TrimmedMean>
        , CoordinateFix
        , ImWrite
    >;
    using This = ImgGrid;
    template<class PTREE>
    ImgGrid(
        PTREE&& ptree
      , const int32_t cv_num_threads
    ) {
        cpt::utility::FilePathChecker checker;
        cv::setNumThreads(cv_num_threads);
        auto opts = cpt::format::make_json ( std::forward<PTREE>( ptree ) ) ;
        buffer_.config(opts);
    }
    

    auto run(
          const std::string& path
        , const int xmn
        , const int ymn
        , const char x_axis_direction
        , const std::string& origin_position
        , const std::string& grid_img_path
        , const std::string& output_dir
        , DebugArgs& debug_args
        , const cv::Point& abs_grid_probe_point
        , const std::string& fname_pos_pat
        , const std::string& spec_name
    ) 
    {
        cpt::msg << "\nprocess " << path << '\n';
        auto buf = this->buffer_;
        buf.y_marker_num = ymn;
        buf.x_marker_num = xmn;
        buf.img_path = path;
        buf.x_axis_direction = x_axis_direction;
        buf.origin_position = origin_position;
        buf.output_dir = output_dir;
        buf.p_debug_args = &debug_args;
        buf.abs_grid_probe_point = abs_grid_probe_point;
        buf.fname_pos_pat = fname_pos_pat;
        buf.spec_name = spec_name;

        if ( !boost::filesystem::exists( path ) )
        {
            std::cout << VDUMP(buf.x_marker_num) << std::endl;
            std::cout << VDUMP(buf.y_marker_num) << std::endl;
            std::cout << VDUMP(buf.marker_x_interval )<< std::endl;
            std::cout << VDUMP(buf.marker_y_interval )<< std::endl;
            std::cout << VDUMP(buf.markers.at(0).rows) << std::endl;
            std::cout << VDUMP(buf.markers.at(1).cols) << std::endl;
        }
        else
        {
            // auto src = im_::imread(path);

            cache_able_section(
                  path + ".dbg.mean.intensity"
                , [&]( auto& bi )
                {
                    cv_mat_serialize_in<float> ( bi, buf.mean );
                }
                , [&] ()
                {
                    buf.src = read_img_to_mat( path );
                    if ( grid_img_path != "" )
                    {
                        buf.grid_img = read_img_to_mat( grid_img_path );
                        buf.has_grid_img = true;
                    }
                    else
                    {
                        buf.has_grid_img = false;
                    }
                    // cv::extractChannel(src, buf.src, src.channels() - 1);
                    // if (src.elemSize1() == 1)
                    //     buf.src.convertTo(buf.src, CV_16U, 257.0);
                    buf.show(
                        buf.v_rawimg
                      , [&buf] { return buf.src; }
                    );
                    PC::Pipeline::run(buf);
                }
                , [&]( auto& bo )
                {
                    cv_mat_serialize_out<float> ( bo, buf.mean );
                }
                , false
            );
        }
        return buf;
    }
    const auto& get_buffer()
    {
        return this->buffer_;
    }
    struct BufferType : public BufferControl<PC, PC2>
    {
        std::string     output_dir              ;
        DebugArgs*      p_debug_args            ;
        cv::Point       abs_grid_probe_point    ;
    };
  private:
    // using BufferType = PC::Buffer;
    BufferType buffer_;
};
struct ImgArgs
{
    std::string                     img_                        ;
    std::string                     probe_id_map_               ;
    boost::property_tree::ptree     img_grid_option_            ;
    const int                       x_marker_num                ;
    const int                       y_marker_num                ;
    cv::Point                       abs_grid_point_             ;
    char                            x_axis_direction_           ;
    std::string                     origin_position_            ;
    std::string                     grid_img_path               ;
    std::string                     output_dir_                 ;
};

template<class OPTION_PARSER>
struct Main
{
    using Summarizer = Summarize;
    Main( OPTION_PARSER&& args )
    : args_ ( std::forward<OPTION_PARSER> ( args ) ) 
    {}
    template<class CONV>
    static auto array_formatter( const cfcs_::Array& arr, const std::string& file, CONV conv )
    {
        return conv( arr, file );
    }
    auto get_array_formatter( const std::string& format )
    {
        std::function<void(const cfcs_::Array&, const std::string& file)> res;
        if ( format == "cel" )
        {
            res = [](const cfcs_::Array& arr, const std::string& file)
            {
                array_formatter( arr, file, cf_::converter::Array2Cel{} );
            };
        }
        else if ( format == "cen")
        {
            res = [](const cfcs_::Array& arr, const std::string& file)
            {
                array_formatter( arr, file, cf_::converter::Array2Cen{} );
            };
        }
        else if ( format == "il" )
        {
            res = [](const cfcs_::Array& arr, const std::string& file)
            {
                array_formatter( arr, file, cf_::converter::Array2Il{} );
            };
        }
        else
        {
            throw std::runtime_error("unknown format : " + format );
        }
        return res;
    }
    template<class ARGS, class ID_MAPPER>
    auto run_improc_pipe(  
          ARGS&& args
        , ID_MAPPER& probe_id_mapper
        , DebugArgs& debug_args
        , const std::string& fname_pos_pat 
        , const std::string& spec_name
    )
    {
        ImgGrid     img_grid   { args.img_grid_option_, 4 };
        auto&& buf     ( img_grid.run( 
              args.img_
            , args.x_marker_num
            , args.y_marker_num
            , args.x_axis_direction_
            , args.origin_position_
            , args.grid_img_path
            , args.output_dir_
            , debug_args
            , args.abs_grid_point_
            , fname_pos_pat
            , spec_name
        ) );
        // buf.abs_grid_probe_point    = { 
        //       args.abs_grid_point_.x - 2
        //     , args.abs_grid_point_.y - 2 
        // };
        return buf;
    }
    template<class BUF>
    decltype(auto) run_improc_pipe2( BUF&& buf )
    {
        buf = ImgGrid::PC2::Pipeline::run(buf);
        return buf;
    }
    auto get_stitched_map( const std::string& spec_name, const std::string& stitched_map_path )
    {
        using SMTsvParseTrait = cpt::format::TraitPrototype<
            std::tuple
            , cpt::format::PMT<0, std::string> // label
            , cpt::format::PMT<1, std::size_t> // x
            , cpt::format::PMT<2, std::size_t> // y
        >;
        using SMTsvParser = cpt::format::TupleParser< SMTsvParseTrait >;

        SMTsvParser parser("\t");
        std::map<std::string, std::pair< std::size_t, std::size_t>> res;
        std::ifstream fin ( stitched_map_path );
        std::string line;
        while ( std::getline( fin, line ) )
        {
            auto tup ( parser( line ) );
            res.emplace( 
                std::get<0>( tup )
                , std::make_pair(
                      std::get<1> ( tup ) - ( spec_name == "denali" ? 2 : 0 )
                    , std::get<2> ( tup ) - ( spec_name == "denali" ? 2 : 0 )
                )
            );
        }
        return res;
    }
    auto get_images_args( 
          const std::string& image_arg_file 
        , const std::string& probe_id_mapper
        , const boost::property_tree::ptree& option
        , const std::map< std::string
            , std::pair< std::size_t, std::size_t > 
        > & sm
    )
    {
        std::vector<ImgArgs> res;
        auto&& jimgs = cpt::format::read_json( image_arg_file );
        for ( auto&& jimg : jimgs.get_list("images") )
        {
            auto&& label = jimg.first;
            auto&& jimg_arg = cpt::format::make_json( jimg.second );
            auto&& abs_LB_marker_pos = sm.at( label );
            ImgArgs image {
                  jimg_arg.template get<std::string>("path")
                , probe_id_mapper
                , option
                , (const int)jimg_arg.template get<int>("x_marker_num")
                , (const int)jimg_arg.template get<int>("y_marker_num")
                , cv::Point(
                      (int)abs_LB_marker_pos.first
                    , (int)abs_LB_marker_pos.second
                )
                , jimg_arg.template get<char>("x_axis_direction") 
                , jimg_arg.template get<std::string>("origin_position")
                , jimg_arg.template get_optional<std::string>("grid_img_path").value_or("")
                , jimgs.template get_optional<std::string>("output_dir").value_or(boost::filesystem::temp_directory_path().string() )
            };
            res.emplace_back( std::move( image ) );
        }
        return res;
    }
    template< class JSON>
    std::unique_ptr<ProbeIdMapper> make_probe_id_mapper( JSON&& json )
    {
        auto spec_name = json.template get<std::string>("spec.name");
        if ( spec_name == "denali" )
        {
            return cpt::utility::make_unique(
                new DenaliIdMapper {
                    std::ifstream(
                        json.template get<std::string>( "probe_id_map" )
                    )
                    , json.template get<uint16_t>("chip_grid_height")
                    , json.template get<uint16_t>("chip_grid_width")
                }
            );
        }
        else if ( spec_name == "clariom_s" )
        {
            return cpt::utility::make_unique(
                new ClariomIdMapper{
                    json.template get<uint16_t>("chip_grid_width")
                }
            );
        }
        else if ( spec_name == "zion" )
        {
            return cpt::utility::make_unique(
                new ZionIdMapper {
                    json.template get<uint16_t>("chip_grid_width")
                }
            );
        }
        else throw std::runtime_error("unknown spec: " + spec_name);
    }
    auto operator()()
    {
        auto json ( cpt::format::read_json( args_.config_ ) );
        std::ofstream mout ( args_.output_ );
        using PG = std::decay_t<decltype( make_partial_grid( 
              std::declval<std::unique_ptr< ProbeIdMapper >&>()
            , ImgGrid::BufferType{}
            , std::move(ImgArgs{}.abs_grid_point_)
        ) )>;
        using SG = GetStitchedGridType<PG>;
        std::unique_ptr<ProbeIdMapper> pidm(
            make_probe_id_mapper(json)
        );
        DebugArgs debug_args ( json ) ;
        auto jimage_args = cpt::format::read_json( args_.image_args_ );
        boost::filesystem::path output_dir( 
            jimage_args.template get<std::string>( "output_dir" ) 
        );
        auto images_args = get_images_args( 
              args_.image_args_
            , json.template get<std::string>("probe_id_map")
            , json.get_child("options").root
            , get_stitched_map( 
                  json.template get<std::string>("spec.name") 
                , json.template get<std::string>("stitched_marker") 
            )
        );
        auto array_formatter ( get_array_formatter( args_.output_format_ ) );
        SG stitched_grid{
              json.template get<uint16_t>("chip_grid_height")
            , json.template get<uint16_t>("chip_grid_width")
        };

        GridlineList img_grid_list ( 
              json.template get<std::string>("spec.fname_pos_pat") 
            , json.template get<std::string>("spec.name") 
        );
        Report report; // qc report
        std::vector< typename ImgGrid::BufferType > bufs;
        bufs.reserve( images_args.size() );
        for ( auto&& imga : images_args )
        {
            auto&& buf  = run_improc_pipe( 
                  imga
                , pidm
                , debug_args
                , json.template get<std::string>("spec.fname_pos_pat") 
                , json.template get<std::string>("spec.name") 
            );

            report.process_buf( buf );
            // clean buf
            bufs.emplace_back( std::move( buf ) );
        }
        auto global_mean = background_fix::img_global_factor_handle( bufs );
        for ( std::size_t i (0); i < bufs.size(); i ++ )
        {
            auto&& b                = bufs.at(i);
            auto&& imga             = images_args.at(i);
            b.global_bg_factor      = global_mean;
            b                       = run_improc_pipe2( b );
            auto&& pg               = make_partial_grid(
                  pidm
                , std::move( b )
                , FWD( imga.abs_grid_point_ )
            );
            stitched_grid.emplace_back ( std::move ( pg ) );
            img_grid_list.add_img_gl ( imga.img_, b.gl_x, b.gl_y );
        }
        ImWrite::run_bufs( bufs );
        img_grid_list.dump ( make_ofstream ( 
            output_dir / "gridline.json"
        ));
        report.write( make_ofstream( output_dir / "report.json" ) );
        auto array_data = stitched_grid.template to<cfcs_::Array>();
        array_formatter( array_data, args_.output_ );
    }
  private:
    OPTION_PARSER args_ ;
};
template<class T>
auto make( T&& option )
{
    return Main<T>( std::forward<T>(option) );
}
}}}
