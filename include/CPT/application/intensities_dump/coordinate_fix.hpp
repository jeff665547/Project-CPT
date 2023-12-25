#pragma once
#include <CPT/improc/coordinate_fix.hpp>
namespace cpt{ namespace application{ namespace intensities_dump{


struct CoordinateFix
{
    struct Parameters
    {
        char x_axis_direction           ;
        std::string origin_position            ;
        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
        }
        template <class JSON>
        void config(JSON&& opts)
        {
        }
    };
  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        cpt::improc::coordinate_fix(
              buf.mean
            , buf.stddev
            , buf.pixels
            , buf.cv_mat
            , buf.detail_raw_values
            , buf.roi
            , buf.origin_position
            , buf.x_axis_direction
        );
    }
};

}}}
