/**
 *  @file    CPT/improc/gridding.hpp
 */
#pragma once
#include <opencv2/core/core.hpp>
#include <CPT/improc/util.hpp>
namespace cpt { namespace improc
{


/**
 *  @brief Recognize the grid border of the image.
 *  @details Detail information can see here @ref improc_gridding
 */
template<class FLOAT>
struct Gridding
{
    template <int32_t dim, int32_t lg2nfft>
    auto fit_sinewave(const cv::Mat_<FLOAT>& src, const double max_intvl)
    {
        // 1D projection
        cv::Mat_<FLOAT> data;
        cv::reduce(src, data, dim, CV_REDUCE_AVG, data.depth());
        if (dim == 1)
            cv::transpose(data, data);
    
        // Moving average
        cv::Mat_<FLOAT> ac;
        cv::copyMakeBorder(
            data -= cv::mean(data)
          , ac
          , 0.0
          , 0.0
          , 0.0
          , (1 << lg2nfft) - data.total()
          , cv::BORDER_CONSTANT
        );
        
        // Frequency detection
        cv::Mat_<cv::Point_<FLOAT>> ft;
        cv::dft(ac, ft, cv::DFT_COMPLEX_OUTPUT);
        FLOAT max = 0.0;
        FLOAT loc = 0.0;
        auto last = ft.total() >> 1;
        for (auto i = decltype(last)(ft.total()/max_intvl); i != last; ++i)
        {
            auto val = cv::norm(ft(i));
            if (max < val)
            {
                max = val;
                loc = i;
            }
        }
        double freq = loc / static_cast<FLOAT>(ft.total());
        cpt::msg << "intvl = " << 1.0 / freq << '\n';
    
        // Phase estimation
        cv::Mat_<FLOAT> Phi, w;
        const auto v = 2.0 * CV_PI * freq;
        for (decltype(data.total()) i = 0; i != data.total(); ++i)
        {
            cv::Mat_<FLOAT> phi = (
                decltype(phi)(1, 3)
                <<  std::cos(v * i)
                  , std::sin(v * i)
                  , 1.0
            );
            Phi.push_back(phi);
        }
        cv::transpose(data, data);
        cv::solve(Phi, data, w, cv::DECOMP_NORMAL);
        const double phase = std::atan2(w(0), w(1)) * 0.5 / CV_PI + 0.25;
        cpt::msg << "phase = " << phase << '\n';
        
        // Generate gridlines
        std::vector<FLOAT> anchors;
        int32_t start = std::ceil(phase);
        int32_t end = std::floor(freq * data.total() + phase);
        while (std::round((start - 1 - phase) / freq) > 0) --start;
        while (std::round((end - phase) / freq) < data.total()) ++end;
        for (auto i = start; i != end; ++i)
            anchors.emplace_back((i - phase) / freq);
    
        return anchors;
    }
        
    /**
     *  @brief  Recognize the grid border of the image.
     *  @param  in_src       Input image
     *  @param  max_intvl    Max interval of grid line
     *  @param  v_final      Show grid result
     *  @param  img_path     The filesystem path of raw image.
     *  @param  verbose Set to false if no image shown are needed ( will override other "v_" prefix variable ), else set to true.
     *  @return grid rows, grid cols, grid tiles ( a rectangle set )
     */
    auto operator()( 
          cv::Mat& in_src
        , double max_intvl 
        , int16_t v_final
        , const std::string& img_path
        , bool verbose = true
    )
    {
        auto src = in_src.clone();
        auto x = fit_sinewave<0,18>(src, max_intvl);
        auto y = fit_sinewave<1,18>(src, max_intvl);
    
        std::vector<cv::Rect> tiles;
        for (decltype(y.size()) j = 1; j != y.size(); ++j)
        {
            const auto y1 = std::round(y[j - 1]);
            const auto y2 = std::round(y[j]);
            for (decltype(x.size()) i = 1; i != x.size(); ++i)
            {
                const auto x1 = std::round(x[i - 1]);
                const auto x2 = std::round(x[i]);
                tiles.emplace_back(
                    cv::Point(x1, y1)
                  , cv::Point(x2, y2)
                );
            }
        }
        std::vector<uint32_t> gl_x, gl_y;
        for ( auto&& xi : x ) 
            gl_x.emplace_back ( std::round(xi) );
        for ( auto&& yi : y )
            gl_y.emplace_back ( std::round(yi) );
        // in_tiles = std::move(tiles);
        uint16_t feature_rows = y.size() - 1;
        uint16_t feature_cols = x.size() - 1;
    
        cpt::msg << "feature rows = " << feature_rows << '\n';
        cpt::msg << "feature cols = " << feature_cols << '\n';
    
        // draw gridding result

       
        auto fname = boost::filesystem::path(img_path).stem().string() + "_gridding.tif";
        cv_imshow(
              fname
            , [&in_src, &tiles]
            {
                cv::Mat_<uint16_t> tmp = in_src.clone();
                trim_outlier( tmp, 0, 0.5 );
                cv::normalize( tmp, tmp, 0, 65535, cv::NORM_MINMAX );
                const auto color = cv::Scalar(cmax(tmp) * 0.5);
                for (auto tile: tiles)
                {
                    tile.width  += 1;
                    tile.height += 1;
                    cv::rectangle(tmp, tile, color);
                }
                return tmp;
            }
            , verbose
        ); 
        struct {
            uint16_t feature_rows, feature_cols;
            std::vector<cv::Rect> tiles;
            std::vector<uint32_t> gl_x, gl_y;
        } res { 
              feature_rows
            , feature_cols 
            , std::move( tiles )
            , std::move( gl_x )
            , std::move( gl_y )
        };
        return res;
    }
};
}}
