#pragma once
#include <CPT/application/improc2/pipeline.hpp>
#include <CPT/application/improc2/improc.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
namespace cai_ = cpt::application::improc2;
template<class T>
struct GetParameterImpl { using Result = typename T::Parameters; };

template<class T>
using GetParameter = typename GetParameterImpl<T>::Result;
template<class PIPE_CON1, class PIPE_CON2>
struct BufferControl{};
template<class... T>
struct PipeControl;
template<class... COMP1, class... COMP2>
struct BufferControl<
      PipeControl<COMP1...>
    , PipeControl<COMP2...>
> : public COMP1::Parameters..., public COMP2::Parameters...
{
    bool verbose;
    int32_t v_rawimg;
    int32_t v_result;
    cv::Mat src;
    double theta;
    std::vector<cv::Rect> tiles;
    uint16_t feature_rows;
    uint16_t feature_cols;
    cv::Mat_<float> mean;
    cv::Mat_<float> stddev;
    cv::Mat_<uint16_t> pixels;

    template <class JSON>
    void config(JSON&& opts)
    {
        verbose = opts
            .template get_optional<bool>("verbose.enable")
            .value_or(false);
        v_rawimg = opts
            .template get_optional<decltype(v_rawimg)>("verbose.rawimg")
            .value_or(-1);
        v_result = opts
            .template get_optional<decltype(v_result)>("verbose.result")
            .value_or(-1);

        cpt::msg << "verbose = " << verbose << '\n'
                 << "verbose.rawimg = " << v_rawimg << '\n'
                 << "verbose.result = " << v_result << '\n'
                 ;

        if (verbose)
        {
            // cv::namedWindow ("verbose", cv::WINDOW_NORMAL);
            // cv::resizeWindow("verbose", 1600, 1200); 
            // cv::moveWindow  ("verbose", 40 , 60 );
        }

        cai_::Pipe<
              typename COMP1::Parameters...
            , typename COMP2::Parameters...
        >::run(*this, std::forward<JSON>(opts));
    }

    template <class FUNC, class... ARGS>
    void show(int16_t delay, FUNC&& func, ARGS&&... args)
    {
        if (verbose and delay >= 0)
        {
            auto image = func(args...);
            cv::imshow("verbose", image);
            cv::waitKey(delay);
        }
    }
};
template<class... T>
struct PipeControl
{
    using Pipeline = cai_::Pipe<T...>;
    // using Buffer = cai_::Improc::Buffer<
    //     GetParameter<T>...
    // >;
};

}
}}
