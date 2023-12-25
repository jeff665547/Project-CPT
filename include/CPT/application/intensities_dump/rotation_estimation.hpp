#pragma once
#include <CPT/improc/rotation_estimation.hpp>
#include <CPT/application/intensities_dump/debug_args.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
struct RotationEstimation
{
  public:
    using This = RotationEstimation;
    using FLOAT = float;
    struct Parameters
    {
        std::string img_path;
        cv::Mat grid_img;
        bool has_grid_img;
        // uint32_t sample_width  {1200};
        // uint32_t sample_height {1200};
        
        FLOAT min_theta;
        FLOAT max_theta;
        FLOAT steps;
        int32_t v_edges;
        int32_t v_hough;

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("rotation");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
        {
            min_theta = opts
                .template get_optional<FLOAT>("lower")
                .value_or(-5) + 90;
            max_theta = opts
                .template get_optional<FLOAT>("upper")
                .value_or(5) + 90;
            steps = opts
                .template get_optional<FLOAT>("steps")
                .value_or(1000);
            v_edges = opts
                .template get_optional<decltype(v_edges)>("verbose.edges")
                .value_or(-1);
            v_hough = opts
                .template get_optional<decltype(v_hough)>("verbose.hough")
                .value_or(-1);

            cpt::msg << "rotation.verbose.edges = " << v_edges << '\n';
            cpt::msg << "rotation.verbose.hough = " << v_hough << '\n';
        }

    };
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        // std::cout << "==========re=========" << std::endl;
        // std::cout << buf.src(cv::Rect(
        //     0, 0, 50, 50 ) ) << std::endl;
        cpt::improc::RotationEstimation<FLOAT> re;
        auto fname = boost::filesystem::path(buf.img_path).stem();
#ifdef SAVE_ROTATE
// this is for debug
        // std::cout << buf.src << std::endl;
        cpt::improc::imwrite( 
              fname.string() + ".before_rotate.tif"
            , buf.src
        );
#endif
        cache_able_section(
              buf.img_path + ".dbg.rotate.theta"
            , [&]( auto& bi ) { bi & buf.theta; }
            , [&]() 
            { 
                buf.theta = re.operator()(
                      buf.src
                    , buf.has_grid_img
                    , buf.grid_img
                    , buf.min_theta
                    , buf.max_theta
                    , buf.steps
                    , buf.v_edges
                    , buf.v_hough
                    , buf.verbose
                );
            }
            , [&]( auto& bo ) { bo & buf.theta; }
        );
    }

};

}
}}
