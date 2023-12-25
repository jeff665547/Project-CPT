#pragma once
// #include "utils.hpp"
#include <CPT/improc/segmentation.hpp>

namespace cpt {
namespace application {
namespace intensities_dump {

class Segmentation
{
    using This = Segmentation;
    using FLOAT = double;

  public:
    struct Parameters
    {
        // cv::Size pixel_size; // TODO unused ?
        std::vector<int32_t> cell_margin;
        int32_t v_final;
#ifndef CPT__INTENSITIES_DUMP__AUTO_MARGIN
        cv::Mat_<float> cv_mat;                         // all cv value 
        std::vector<cv::Mat_<int32_t>> detail_raw_values;
#endif

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("segmentation");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
        {
            // auto list = opts.get_list("pixel_size");
            // auto h = list.front();
            // pixel_size.height = h.template as_value<int32_t>();
            // list.pop_front();
            // auto w = list.front();
            // pixel_size.width  = w.template as_value<int32_t>();
            // list.pop_front();
            
            for (auto item: opts.get_list("cell_margin"))
                cell_margin.emplace_back(
                    item.second.template get_value<int32_t>()
                );

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
        cpt::improc::Segmentation s;
        s.operator()(buf.src, buf.Parameters::cell_margin, buf.tiles, buf.Parameters::v_final, buf.verbose );
    }
};

#define CPT__INTENSITIES_DUMP__SEGMENTATION

} // namespace improc2
} // namespace application
} // namespace cpt

