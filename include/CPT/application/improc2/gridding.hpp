#pragma once
// #include "utils.hpp"
// #include <CPT/application/intensities_dump/utility.hpp>
#include <CPT/improc/gridding.hpp>
#include <CPT/view.hpp>
#include <CPT/improc/util.hpp>

namespace cpt {
namespace application {
namespace improc2 {

// namespace caid_ = cpt::application::intensities_dump;
class Gridding
{
    using This = Gridding;
    using FLOAT = double;

  public:
    struct Parameters
    {
        bool adaptive;
        double max_intvl;
        int32_t v_final;
        std::vector<uint32_t> gl_x, gl_y;

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("gridding");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
        {
            max_intvl = opts
                .template get_optional<decltype(max_intvl)>("max_intvl")
                .value_or(100);

            v_final = opts
                .template get_optional<decltype(v_final)>("verbose.final")
                .value_or(-1);

            cpt::msg << "gridding.max_intvl = " << max_intvl << '\n';
            cpt::msg << "gridding.verbose.final = " << v_final << '\n';
        }
    };


  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        // std::cout << "==========gridding=========" << std::endl;
        // std::cout << buf.src(cv::Rect(
        //     0, 0, 50, 50 ) ) << std::endl;
        cpt::improc::Gridding<FLOAT> g;
        auto res = g( 
              buf.src
            , buf.Parameters::max_intvl
            , buf.Parameters::v_final
            , buf.img_path
            , buf.verbose 
        );
        buf.feature_rows = res.feature_rows;
        buf.feature_cols = res.feature_cols;        
        buf.tiles = std::move( res.tiles );
        buf.gl_x = res.gl_x;
        buf.gl_y = res.gl_y;

    }
};

} // namespace improc2
} // namespace application
} // namespace cpt
