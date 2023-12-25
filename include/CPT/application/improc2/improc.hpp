#pragma once
#include "rotation_estimation.hpp"
#include "gridding.hpp"
#include "segmentation.hpp"
#include "pipeline.hpp"
#include "utils.hpp"
#include <CPT/format/json.hpp>
#include <Nucleona/format/hdf5.hpp>
#include <CPT/utility/file_path_checker.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>

namespace cpt {
namespace application {
namespace improc2 {

namespace im = cpt::application::improc2;

/// Trimmed mean method
/** This module is used to calculate the trimmed mean of probe cell
 *  given the region obtained from the ImageGridding component
 *  It will output a tuple of the trimmed mean, trimmed standard deviation,
 *  and the number of pixel intensities for summarization.
 */
class TrimmedMean
{
  public:
    using This = TrimmedMean;
    using ValueType = std::tuple<float, float, uint16_t>;
    struct Parameters
    {
        float pctl[2];
    };
    Parameters m;

  public:
    template <class BUFFER>
    TrimmedMean(BUFFER&& buf)
    {
        m.pctl[0] = buf.This::Parameters::pctl[0];
        m.pctl[1] = buf.This::Parameters::pctl[1];
    }

    template <class MAT>
    auto operator()(MAT&& src, cv::Rect roi)
    {
        cv::Mat_<float> range = src(roi);
        auto lower  = percentiles(range, m.pctl[0]);
        auto upper  = percentiles(range, m.pctl[1]);
        auto pixels = 0;
        auto prev   = 0.0;
        auto mean   = 0.0;
        auto stddev = 0.0;
        for (decltype(range.total()) i = 0; i != range.total(); ++i)
        {
            auto value = range.template at<float>(i);
            if (value >= lower and value <= upper)
            {
                prev    = mean;
                mean   += (value - mean) / (++pixels);
                stddev += (value - mean) * (value - prev);
            }
        }
        stddev = std::sqrt(stddev / (pixels - 1));

        return ValueType(mean, stddev, pixels);
    }

    template <class MAT>
    float percentiles(MAT&& roi, const float p)
    {
        auto roi__ = roi.clone();
        int32_t offset = std::round(p * roi__.total());
        std::nth_element(
            roi__.begin()
          , roi__.begin() + offset
          , roi__.end()
        );
        return roi__(offset);
    }
};

template <class POLICY>
class Summary
{
    using This = Summary<POLICY>;

  public:
    struct Parameters
         : public POLICY::Parameters
    {
        double  v_scale;
        int32_t v_mean;
        int32_t v_std;
        int32_t v_cv;

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("summary");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
        {
            auto list = opts.get_list("percentiles");
            this->pctl[0] = list.front().template as_value<double>();
            list.pop_front();
            this->pctl[1] = list.front().template as_value<double>();

            v_scale = opts
                .template get_optional<decltype(v_scale)>("verbose.scale")
                .value_or(-1);
            v_mean = opts
                .template get_optional<decltype(v_mean)>("verbose.mean")
                .value_or(-1);
            v_std = opts
                .template get_optional<decltype(v_std)>("verbose.std")
                .value_or(-1);
            v_cv = opts
                .template get_optional<decltype(v_cv)>("verbose.cv")
                .value_or(-1);

            cpt::msg << "trimmed_mean.verbose.scale = " << v_scale << '\n'
                     << "trimmed_mean.verbose.mean = "  << v_mean  << '\n'
                     << "trimmed_mean.verbose.std = "   << v_std   << '\n'
                     << "trimmed_mean.verbose.cv = "    << v_cv    << '\n';
        }
    };

  public:
    template <class BUFFER>
    static void run(BUFFER& buf)
    {
        auto& src = buf.src;
        auto& tiles = buf.tiles;

        cv::Mat_<float>   mean  (cv::Size(0, 0), CV_32FC1);
        cv::Mat_<float>   stddev(cv::Size(0, 0), CV_32FC1);
        cv::Mat_<float>   pixels(cv::Size(0, 0), CV_16UC1);
        std::vector<cv::Mat_<int32_t>> detail_raw_values;
        POLICY policy(buf);
        for (auto& roi: tiles)
        {
            auto res = policy(src, roi);
            mean  .push_back(std::get<0>(res));
            stddev.push_back(std::get<1>(res));
            pixels.push_back(std::get<2>(res));
            detail_raw_values.push_back( src(roi) );
        }
        buf.mean   = std::move(mean  .reshape(1, buf.feature_rows));
        buf.stddev = std::move(stddev.reshape(1, buf.feature_rows));
        buf.pixels = std::move(pixels.reshape(1, buf.feature_rows));
        buf.detail_raw_values = std::move( detail_raw_values );

        buf.show(
            buf.This::Parameters::v_mean
          , [&buf] { return buf.mean / 65535.0; }
        );
        buf.show(
            buf.This::Parameters::v_std
          , [&buf] { return buf.stddev / 65535.0; }
        );
        buf.show(
            buf.This::Parameters::v_cv
          , [&buf] { return buf.stddev / buf.mean; }
        );
    }
};

class ROIDetection
{
  public:
    using This = ROIDetection;

    struct Parameters
    {
        cv::Mat_<uint8_t> marker;
        bool enable;
        int32_t v_scale;
        int32_t v_mean;
        int32_t v_std;
        int32_t v_cv;

        template <class BUFFER, class JSON>
        static void run(BUFFER&& buf, JSON&& json)
        {
            auto&& opts = json.get_child("roi_detection");
            buf.This::Parameters::config(opts);
        }
        template <class JSON>
        void config(JSON&& opts)
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
            v_std = opts
                .template get_optional<decltype(v_std)>("verbose.std")
                .value_or(-1);
            v_cv = opts
                .template get_optional<decltype(v_cv)>("verbose.cv")
                .value_or(-1);

            cpt::msg << "roi_detection.verbose.scale = " << v_scale << '\n'
                     << "roi_detection.verbose.mean = "  << v_mean  << '\n'
                     << "roi_detection.verbose.std = "   << v_std   << '\n'
                     << "roi_detection.verbose.cv = "    << v_cv    << '\n';
        }
    };

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        if (!buf.This::Parameters::enable)
            return;

        cv::Mat_<uint8_t> bw;
        cv::normalize(buf.mean, bw, 0, 255.0, cv::NORM_MINMAX, bw.depth());
        cv::threshold(bw, bw, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
        // binarization

        cv::Mat_<float> scores(
            bw.cols - buf.marker.cols + 1
          , bw.rows - buf.marker.rows + 1
        );
        cv::matchTemplate(bw, buf.marker, scores, CV_TM_CCORR_NORMED);
        // matching template
        
        const int32_t texp = 0;
        const int32_t bexp = 0;
        const int32_t lexp = 0;
        const int32_t rexp = 0;

        std::vector<cv::Point> offsets;
        for (auto r = 0; r != scores.rows; ++r)
            for (auto c = 0; c != scores.cols; ++c)
                if (scores(r, c) >= 0.85) // matching threshold
                    offsets.emplace_back(c, r);
        // compare to threshold
        
        if (buf.This::Parameters::enable)
        {
            if (!offsets.empty())
            {
                std::cerr << offsets.size() << " markers are detected\n";
    
                auto roi = cv::boundingRect(offsets);
                roi.x -= lexp;
                roi.y -= rexp;
                roi.width  += buf.marker.cols - 1 + lexp + rexp;
                roi.height += buf.marker.rows - 1 + texp + bexp;
    
                std::cerr << roi << '\n';
                buf.mean = buf.mean(roi);
                buf.stddev = buf.stddev(roi);
                buf.pixels = buf.pixels(roi);
                buf.feature_cols = roi.width;
                buf.feature_rows = roi.height;
                // bounded box
            }
            else
            {
                std::cerr << "No markers are detected\n";
            }
        }
    
        buf.show(
            buf.This::Parameters::v_mean
          , [&buf]
            {
                auto tmp = buf.mean.clone();
                cv::normalize(tmp, tmp, 0.0, 1.0, cv::NORM_MINMAX, CV_32F);
                return tmp;
                // return buf.mean / 65535.0;
            }
        );
        buf.show(
            buf.This::Parameters::v_std
          , [&buf] { return buf.stddev / 65535.0; }
        );
        buf.show(
            buf.This::Parameters::v_cv
          , [&buf] { return buf.stddev / buf.mean; }
        );
    }
};

class Improc
{
  public:
    template <class... PARAMETERS>
    struct Buffer
         : public PARAMETERS...
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

            Pipe<PARAMETERS...>::run(*this, std::forward<JSON>(opts));
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
    using BufferType = Buffer<
        RotationEstimation::Parameters
      , RotationCalibration::Parameters
      , Gridding::Parameters
      , Segmentation::Parameters
      , Summary<TrimmedMean>::Parameters
      , ROIDetection::Parameters
    >;

  private:
    uint16_t scanning_rows;
    uint16_t scanning_cols;
    BufferType buffer;
    bool do_stitch;

  public:
    Improc(
        const std::string& path_to_options
      , const int32_t cv_num_threads
    ) {
        cpt::utility::FilePathChecker checker;
        cv::setNumThreads(cv_num_threads);
        checker(path_to_options);
        checker.verify(); 

        /// configure global options
        auto json = cpt::format::read_json(path_to_options);
        auto opts = json.get_child("options");
        scanning_rows = opts.get<decltype(scanning_rows)>("rows");
        scanning_cols = opts.get<decltype(scanning_cols)>("cols");
        buffer.marker = this->load_marker(opts.get<std::string>("marker"));
        buffer.config(opts);
        do_stitch = opts.get<bool>("stitch");
    }

    // void run(
    //     const std::string& path_to_input
    //   , const std::string& path_to_output
    // ) {
    //     cpt::utility::FilePathChecker checker;
    //     checker(path_to_input);
    //     checker.verify();

    //     auto json = cpt::format::read_json(path_to_input);
    //     auto name = json.get<std::string>("run_name");
    //     auto list = json.get_list("images");
    //     std::vector<bfs::path> input_paths(scanning_rows * scanning_cols);
    //     for (auto item: list)
    //         input_paths[std::stoi(item.first)]
    //             = item.second.get_value<std::string>();
    //     for (auto& path: input_paths)
    //         checker(path);
    //     auto output_dir = json.get<bfs::path>("output_dir");
    //     checker(output_dir);
    //     checker.verify();

    //     json = cpt::format::make_json(); 

    //     // process for each image
    //     std::vector<cv::Mat_<float>> results;
    //     for (auto& path: input_paths)
    //     {
    //         cpt::msg << "\nprocess " << path << '\n';
    //         auto src = im::imread(path);
    //         auto buf = buffer;
    //         cv::extractChannel(src, buf.src, src.channels() - 1);
    //         if (src.elemSize1() == 1)
    //             buf.src.convertTo(buf.src, CV_16U, 257.0);

    //         buf.show(
    //             buf.v_rawimg
    //           , [&buf] { return buf.src; }
    //         );

    //         im::Pipe<
    //             im::RotationEstimation
    //           , im::RotationCalibration
    //           , im::Gridding
    //           , im::Segmentation
    //           , im::Summary<TrimmedMean>
    //           , im::ROIDetection
    //         >::run(buf);

    //         if (do_stitch)
    //             results.emplace_back(buf.mean);

    //         // TODO
    //         auto fname = output_dir / path.filename()
    //                                       .replace_extension("mat")
    //                                       .string();

    //         this->export_matrix(fname, buf.mean);
    //     }

    //     if (do_stitch)
    //     {
    //         cpt::msg << "\nstitch images ... ";
    //         auto result = this->stitch(results);
    //         cpt::msg << "done\n";
    //         buffer.show(
    //             buffer.v_result
    //           , [&result]{ return result / 65535.0; }
    //         );
    //         auto path_to_result = output_dir / (name + ".bmp");
    //         auto path_to_matrix = output_dir / (name + ".mat");
    //         
    //         cv::Mat tmp;
    //         result.convertTo(tmp, CV_8UC1, 1.0 / 257.0);

    //         im::imwrite(path_to_result.string(), tmp);
    //         this->export_matrix(path_to_matrix, result);
    //         json.add("rows", result.rows);
    //         json.add("cols", result.cols);
    //         json.add("img" , path_to_result.string()); // TODO To Alex why use path.native ?
    //         json.add("mat" , path_to_matrix.string());
    //         json.add("cen" , "/path/to/cen");
    //         
    //         auto now = boost::posix_time::second_clock::local_time();
    //         json.add("date", boost::posix_time::to_simple_string(now).c_str());
    //     }

    //     std::ofstream os(path_to_output);
    //     json.dump(os);
    //     os.close();
    // }

    void export_matrix(const bfs::path& path_to_matrix, const cv::Mat_<float>& mat)
    {
        std::cerr << "export to " << path_to_matrix.string() << '\n';
        std::ofstream os(path_to_matrix.string());
        for (auto r = 0; r != mat.rows; ++r)
        {
            os << mat(r, 0);
            for (auto c = 1; c < mat.cols; ++c)
                os << "\t" << mat(r, c);
            os << '\n';
        }
        os.close();
    }

    cv::Mat_<float> stitch(const std::vector<cv::Mat_<float>>& images)
    {
        auto marker_rows = buffer.marker.rows;
        auto marker_cols = buffer.marker.cols;
        auto image_rows = images.front().rows;
        auto image_cols = images.front().cols;

        int32_t total_rows = 0;
        for (auto r = 0; r != scanning_rows; ++r)
            total_rows += images[scanning_cols * r].rows;

        int32_t total_cols = 0;
        for (auto c = 0; c != scanning_cols; ++c)
            total_cols += images[c].cols;

        cv::Mat_<float> full(
            total_rows - marker_rows * (scanning_rows - 1)
          , total_cols - marker_cols * (scanning_cols - 1)
        );
        full = 0.0;

        int32_t row_offset = 0;
        for (auto r = 0; r != scanning_rows; ++r)
        {
            int32_t col_offset = 0;
            for (auto c = 0; c != scanning_cols; ++c)
            {
                auto& res = images[c + scanning_cols * r];
                cv::Rect roi(
                    col_offset
                  , row_offset
                  , res.cols
                  , res.rows
                );
                res.copyTo(full(roi));

                col_offset += image_cols - 10;
            }
            row_offset += image_rows - 10;
        }

        return full;
    }

  // private:
  public:
    static cv::Mat_<uint8_t> load_marker(const bfs::path& path_to_marker)
    {
        if (!bfs::exists(path_to_marker))
        {
            cpt::fatal << path_to_marker << " not found!";
            exit(1);
        }
        cpt::msg << "found " << path_to_marker << '\n';

        std::ifstream is(path_to_marker.string());
        cv::Mat_<uint8_t> marker = decltype(marker)::zeros(10, 10);
        char c;
        decltype(cv::Mat_<uint8_t>{}.total()) i = 0;
        while (is >> c and i != marker.total())
        {
            if (c == 'X')
                marker(i++) = 255;
            else if (c == '.')
                marker(i++) = 0;
        }
        is.clear();
        is.close();

        return marker;
    }
};

} // namespace improc2
} // namespace application
} // namespace cpt
