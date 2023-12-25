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
#include <CPT/view.hpp>
#include <CPT/improc/util.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
namespace ba_ = boost::adaptors;
struct SrcROIInfer : public cpt::improc::SrcROIInfer
{
    using This = SrcROIInfer;
    struct Parameters
    {
        bool enable;
        int16_t v_final;
        // boost::sliced_range<std::vector<uint32_t>> gl_x;
        // boost::sliced_range<std::vector<uint32_t>> gl_y;

        template<class BUFFER, class JSON>
        static void run ( BUFFER&& buf, JSON&& json )
        {
            auto&& opts = json.get_child("src_r_o_i_infer");
            buf.This::Parameters::config( opts );
        }
        template<class JSON>
        void config ( JSON&& opts )
        {
            enable = opts
                .template get_optional<decltype(enable)>("enable")
                .value_or(false);
            v_final = opts
                .template get_optional<decltype(v_final)>("verbose.final")
                .value_or(-1);
        }
    };
    template<class BUFFER>
    static void run ( BUFFER&& buf )
    {
        This src_r_o_i_infer;
        // img_debug( buf, true );
        
        if ( buf.Parameters::enable )
        {
            buf.src = src_r_o_i_infer(
                      buf.roi
                    , buf.src
                    , buf.gl_x
                    , buf.gl_y
                    , buf.tiles
                    , buf.Parameters::enable
                    , buf.Parameters::v_final
                    , buf.verbose
                )
            ;
        }
    }

};

}}}
