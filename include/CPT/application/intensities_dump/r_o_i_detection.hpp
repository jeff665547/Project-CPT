#pragma once 
#include <CPT/improc/r_o_i_detection.hpp>
#include <CPT/format/json.hpp>
#include <CPT/improc/util.hpp>
#include <Nucleona/algo/split.hpp>
#include <CPT/utility/assert.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
// namespace cai_ = cpt::application::improc2;

struct ROIDetectionWithLayout : public cpt::improc::ROIDetection// : public cai_::ROIDetection
{
    using This = ROIDetectionWithLayout;
    struct Parameters
    {
        using This = Parameters;
        cv::Rect roi;
        uint32_t marker_x_interval;
        uint32_t marker_y_interval;

        uint32_t x_marker_num;
        uint32_t y_marker_num;

        bool enable;
        int32_t v_scale;
        int32_t v_mean;
        int32_t v_std;
        int32_t v_cv;
        int32_t v_mean_trimmed;
        int32_t v_mean_binarized;
        int32_t v_mean_score;
        int32_t v_layout_score;
        int32_t v_marker_check;
        std::vector<cv::Mat_<uint8_t>> markers;
        bool roi_qc_fail { false };

        static cv::Mat_<uint8_t> load_marker(const bfs::path& path_to_marker)
        {
            if (!bfs::exists(path_to_marker))
            {
                cpt::fatal << path_to_marker << " not found!";
                exit(1);
            }
            cpt::msg << "found " << path_to_marker << '\n';

            cv::Mat_<uint8_t> marker ;// = decltype(marker)::zeros(10, 10);
            std::ifstream is(path_to_marker.string());
            std::string line;
            int cols = 0;
            int rows = 0;
            while( std::getline( is, line ) )
            {
                auto marks = nucleona::algo::split( line, " " );
                if( cols == 0 ) cols = marks.size();
                else cpt::utility::throw_if( cols != marks.size() );
                for ( auto&& c : marks )
                {
                    if( c == "X" ) marker.push_back( (uint8_t)255 );
                    else if ( c == "." ) marker.push_back( (uint8_t)0 );
                    else throw std::runtime_error( "unknown marker character: " + c );
                }
                rows ++;
            }
            marker = marker.reshape(1, rows );

            is.clear();
            is.close();

            return marker;
        }

        template<class BUFFER, class JSON>
        static void run( BUFFER&& buf, JSON&& json )
        {
            auto m_x_i = json.template get<uint32_t>("marker_x_interval");
            auto m_y_i = json.template get<uint32_t>("marker_y_interval");
            
            for ( auto&& m : json.get_list("markers") )
            {
                auto jm = cpt::format::make_json ( m.second );
                buf.markers.emplace_back ( std::move ( 
                    load_marker( jm.template as_value<std::string>() )
                ) );
            }

            auto&& opts = json.get_child("roi_detection_with_layout");
            buf.This::Parameters::config(opts, m_x_i, m_y_i);
        }

        template<class JSON>
        void config( JSON&& opts, uint32_t m_x_i, uint32_t m_y_i )
        {
            enable = opts
                .template get_optional<decltype(enable)>("enable")
                .value_or(false);

            v_scale = opts
                .template get_optional<decltype(v_scale)>("verbose.scale")
                .value_or(-1);
            v_mean = opts
                .template get_optional<decltype(v_mean)>("verbose.mean")
                .value_or(-1);
            v_mean_trimmed = opts
                .template get_optional<decltype(
                    v_mean_trimmed
                )>( "verbose.mean_trimmed" )
                .value_or(-1);
            v_mean_binarized = opts
                .template get_optional<decltype(
                    v_mean_binarized
                )>( "verbose.mean_binarized" )
                .value_or(-1);
            v_mean_score = opts.template get_optional<decltype(
                v_mean_score 
            )>( "verbose.mean_score" ).value_or(-1);

            v_layout_score = opts.template get_optional<decltype(
                v_layout_score 
            )>( "verbose.layout_score" ).value_or(-1);

            v_std = opts
                .template get_optional<decltype(v_std)>("verbose.std")
                .value_or(-1);
            v_cv = opts
                .template get_optional<decltype(v_cv)>("verbose.cv")
                .value_or(-1);
            v_marker_check = opts
                .template get_optional<decltype(v_marker_check)>("verbose.marker_check")
                .value_or(-1);

            cpt::msg << "roi_detection.verbose.scale            = "  << v_scale << '\n'
                     << "roi_detection.verbose.mean             = "  << v_mean  << '\n'
                     << "roi_detection.verbose.std              = "  << v_std   << '\n'
                     << "roi_detection.verbose.cv               = "  << v_cv    << '\n'
                     << "roi_detection.verbose.mean_trimmed     = "  << v_mean_trimmed      << '\n'
                     << "roi_detection.verbose.mean_binarized   = "  << v_mean_binarized    << '\n'
                     << "roi_detection.verbose.mean_score       = "  << v_mean_score        << '\n'
                     << "roi_detection.verbose.layout_score     = "  << v_layout_score      << '\n'
                     << "roi_detection.verbose.marker_check     = "  << v_marker_check      << '\n'
            ;

            marker_x_interval   = m_x_i;
            marker_y_interval   = m_y_i;
        }
    };
    template<class BUFFER>
    static void run ( BUFFER&& buf )
    {
        This r_o_i_detection;
        auto fname = boost::filesystem::path(buf.img_path).stem();
#ifdef SAVE_ROI
        cpt::improc::imwrite(
              fname.string() + ".before_roi.png"
            , buf.mean
            , 0.04
        );
#endif
            auto res = r_o_i_detection(
                  buf.mean
                , buf.stddev
                , buf.pixels
                , buf.cv_mat
                , buf.detail_raw_values
                , buf.This::Parameters::roi_qc_fail
                , buf.x_marker_num
                , buf.y_marker_num
                , buf.marker_x_interval
                , buf.marker_y_interval
                , buf.markers
                , buf.img_path
                , buf.This::Parameters::enable
                , buf.This::Parameters::v_mean_trimmed
                , buf.This::Parameters::v_mean_binarized
                , buf.This::Parameters::v_mean_score
                , buf.This::Parameters::v_layout_score
                , buf.This::Parameters::v_marker_check
                , buf.This::Parameters::v_mean
                , buf.This::Parameters::v_std
                , buf.This::Parameters::v_cv
                , buf.verbose
            );
            if ( buf.This::Parameters::enable )
            {
                buf.feature_cols = res.feature_cols;
                buf.feature_rows = res.feature_rows;
                buf.roi = res._roi;
                
            }
#ifdef SAVE_ROI
        cpt::improc::imwrite(
              fname.string() + ".after_roi.png"
            , buf.mean
            , 0.04
        );
#endif
    }
};

}
}}
