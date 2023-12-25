#pragma once
#include <CPT/improc/middle_segmentation.hpp>

namespace cpt {
namespace application {
namespace intensities_dump {

class MiddleSegmentation
{
    using This = MiddleSegmentation;
    using FLOAT = double;

  public:
    struct Parameters
    {
        int16_t tile_width;
        int16_t tile_height;
        int32_t v_final;
#ifndef CPT__INTENSITIES_DUMP__AUTO_MARGIN
        cv::Mat_<float> cv_mat;                         // all cv value 
        std::vector<cv::Mat_<int32_t>> detail_raw_values;
#endif

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("middle_segmentation");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
        {
            tile_width = opts.template get<decltype(tile_width)>("tile_width");
            tile_height = opts.template get<decltype(tile_height)>("tile_height");
            v_final = opts
                .template get_optional<decltype(v_final)>("verbose.final")
                .value_or(-1);
            
            cpt::msg << "segmentation.verbose.final = " << v_final << '\n';
        }
    };

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        cpt::improc::MiddleSegmentation s;
        s.operator()(
              buf.src
            , buf.Parameters::tile_width
            , buf.Parameters::tile_height
            , buf.tiles
            , buf.img_path
            , buf.Parameters::v_final
            , buf.verbose 
        );
    }
};

#define CPT__INTENSITIES_DUMP__SEGMENTATION

} // namespace improc2
} // namespace application
} // namespace cpt

