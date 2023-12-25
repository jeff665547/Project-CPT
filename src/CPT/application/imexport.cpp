#include <opencv2/opencv.hpp>
#include "CPT/application/imexport/pipeline.hpp"
#include "CPT/application/imexport/filters.hpp"
#include "CPT/application/imexport/utils.hpp"
#include "CPT/application/imexport/intensity.hpp"
#include <CPT/format/json.hpp>
#include <CPT/format/hdf5.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <set>
#include <numeric>
#include <random>

namespace improc {

/// Rotation bias estimation
/** This module is used to estimate the rotation bias of the microarray image
 *  by using the Hough transform algorithm. Making a histogram plot of the
 *  transformed data and finding the bin of the maximum number of count can
 *  obtain the optimal rotation bias estimation
 */
class RotationAngleEstimation
{
  using FLOAT = double;

  public:
    struct Parameters
    {
        double angle;
        FLOAT tmin;
        FLOAT tmax;
        FLOAT tres;

        Parameters(void) = default;
    };

  private:
    struct UnitVector
    {
        FLOAT angle;
        FLOAT cos_angle;
        FLOAT sin_angle;

        UnitVector(FLOAT angle)
          : angle(angle)
          , cos_angle(std::cos(angle * CV_PI / 180.0))
          , sin_angle(std::sin(angle * CV_PI / 180.0))
        {}
        FLOAT rho(unsigned x, unsigned y) const
        {
            return x * cos_angle + y * sin_angle;
        }
    };

  public:
    template <class DB>
    static void run(DB& db)
    {
        auto& src = db.image;
        const auto ncols = src.cols;
        const auto nrows = src.rows;
        const auto tmin = db.tmin;
        const auto tmax = db.tmax;
        const auto tres = db.tres;
        const FLOAT rmax = std::sqrt(ncols * ncols + nrows * nrows);
        const FLOAT rmin = -ncols;
        const FLOAT rbins = rmax - rmin + 1;

        std::vector<UnitVector> unitvecs;
        for (auto t = tmin; t < tmax; t += tres)
            unitvecs.emplace_back(t);

        auto tmp = Pipe<
            Normalization<cv::NORM_MINMAX, uint8_t>
          , EdgeDetection<double_(0.8)>
        >::run(src.clone());

        // cv::imshow("tmp", tmp);
        // cv::waitKey(0);
        
        cv::Mat_<FLOAT> hist(unitvecs.size(), rbins);
        hist = 0.0;
        for (auto y = 0; y < nrows; ++y)
            for (auto x = 0; x < ncols; ++x)
                if (tmp.template at<uint8_t>(y, x) > 127 and (std::rand() & 0x8) == 0)
                    for (auto& u: unitvecs)
                        hist(
                            static_cast<int32_t>(std::round((u.angle - tmin) / tres))
                          , static_cast<int32_t>(std::round(u.rho(x, y) - rmin))
                        ) += 1.0;

        double val;
        cv::Point loc;
        cv::minMaxLoc(hist, nullptr, &val, nullptr, &loc);
        double angle = loc.y * db.tres + db.tmin;
        auto&& offset = (angle > 135.0)? 180.0
                      : (angle >  45.0)?  90.0: 0.0;

        db.angle = angle - offset;
        std::cerr << "angle = " << db.angle << '\n';

        // improc::Normalization<cv::NORM_MINMAX, FLOAT>::run(hist);
        // cv::line(hist, cv::Point(0, loc.y), cv::Point(hist.cols-1, loc.y), cv::Scalar(1.0));
        // cv::imshow("tmp", hist); 
        // cv::waitKey(0);
    }
};

/// Rotation calibration
/** Calibrating the image based on the rotation estimation
 *  obtained from RotationAngleEstimation component
 */
class RotationAngleCalibration
{
  public:
    template <class DB>
    static void run(DB& db)
    {
        imrotate(db.image, db.angle);
    }
};

/// Feature size detection
/** This module is used to measure the fundamental feature size of the microarray.
 *  The detected feature size would be used in ImageGridding component.
 *  The method is described as follows. First, approximating the contours as rectangles,
 *  and get a rough estimation of feature diameter. Then, applying the template matching
 *  algorithm to find the optimal feature size exhaustively.
 *  Notice that, this detection method does not work well in case of extremely
 *  small feature size. Please set the value FeatureSizeDetection2::Parameters::scale
 *  explicitly to bypass this module.
 *  Currently, we found that the sinusoidal regression can achieve this part, together with
 *  the gridding process. So, this module will be deprecated if we finish the better
 *  detection algorithma.
 */ 
class FeatureSizeDetection2 //TODO rename
{
  public:
    struct Parameters
    {
        double scale; // fundamental feature size
    };

  public:
    template <class DB>
    static void run(DB& db)
    {
        // Preprocessing
        auto tmp = Pipe<
            Normalization<cv::NORM_MINMAX, int16_t>
          , improc::Blur<3>
          , improc::SharpenFilter<double_(50.0)>
          , improc::MedianBlur<3>
          , Normalization<cv::NORM_MINMAX, uint8_t>
          , improc::Binarization<CV_THRESH_OTSU | CV_THRESH_BINARY>
        >::run(db.image.clone());
        // cv::imshow("test", tmp);
        // cv::waitKey(0);

        if (db.scale < 0.0)
        {
            std::cerr << "Detect feature size\n";
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(
                tmp.clone()
              , contours
              , hierarchy
              , ::CV_RETR_EXTERNAL
              , ::CV_CHAIN_APPROX_NONE
              , cv::Point(0, 0)
            );
            std::vector<float> wvalues;
            std::vector<float> hvalues;
            for (auto& contour: contours)
            {
                auto rect = cv::boundingRect(contour);
                auto area = static_cast<double>(rect.area());
                auto ratio = static_cast<double>(rect.width) / rect.height;
                if (area > 8 and (ratio < 1.1 and 1/ratio < 1.1)) //TODO min_feature_size, min_diff_aspect_ratio
                {
                    wvalues.emplace_back(rect.width);
                    hvalues.emplace_back(rect.height);
                    cv::rectangle(tmp, rect, cv::Scalar(cmax(tmp)/2));
                }
            }
            size_t xoffset = wvalues.size() >> 1;
            std::nth_element(
                wvalues.begin()
              , wvalues.begin() + xoffset
              , wvalues.end()
            );
            size_t yoffset = hvalues.size() >> 1;
            std::nth_element(
                hvalues.begin()
              , hvalues.begin() + yoffset
              , hvalues.end()
            );
            int width = ( !wvalues.empty() and !hvalues.empty() )
                      ? ( std::min(wvalues[xoffset], hvalues[yoffset]) )
                      : ( !wvalues.empty() and  hvalues.empty() )
                      ? ( std::max(3.0f, wvalues[yoffset]) )
                      : (  wvalues.empty() and !hvalues.empty() )
                      ? ( std::max(3.0f, hvalues[xoffset]) )
                      : 3.0f;
            
            float best_score = 0;
            float best_scale = 0;
            while (width > 0)
            {
                cv::Mat templ = imresize(
                    db.templ.clone()
                  , static_cast<double>(width) / 90.0
                );
                cv::Mat_<float> scores(
                    tmp.cols - templ.cols + 1
                  , tmp.rows - templ.rows + 1
                );
                cv::matchTemplate(tmp, templ, scores, CV_TM_CCORR_NORMED);
                
                double score;
                cv::minMaxLoc(scores, nullptr, &score);
                std::cerr << width << " -> " << score << '\n';
                if (best_score < score)
                {
                    best_score = score;
                    best_scale = width;
                }
                else break;

                width--;
            }
            db.scale = best_scale;
        }

        std::cerr << "scale = " << db.scale << '\n';
    }
};

/// Image gridding
/** This module is used to find the grid points of the microarray,
 *  and generate the regions of interest for pixel intensity extraction
 *  The idea is based on peak analysis of the 1-D projection of pixel intensities.
 *  The implementation is basically followed a Python package named PeakUtils.
 */
class ImageGridding
{
  public:
    static auto find_peaks(
        const std::vector<float>& hist
      , const int32_t radius
    ) {
        std::vector<size_t> cands;
        std::vector<bool> peak(hist.size(), false);

        for (int i = 1; i < hist.size() - 1; ++i)
        {
            if (hist[i-1] >= hist[i] and hist[i] <= hist[i+1])
            {
                cands.emplace_back(i);
                peak.at(i) = true;
            }
        }

        std::stable_sort(
            cands.begin()
          , cands.end()
          , [&hist](const auto& lhs, const auto& rhs)
            { return hist[lhs] <= hist[rhs]; }
        );

        for (auto& i: cands)
        {
            if (peak[i])
            {
                size_t j = (i > radius)? i - radius: 0;
                size_t e = (i + radius < hist.size())? i + radius: hist.size();
                while (j != e)
                    peak.at(j++) = false;
                peak.at(i) = true;
            }
        }

        std::vector<uint16_t> peak_inds;
        for (auto i = 0ul; i != peak.size(); ++i)
            if (peak[i])
                peak_inds.emplace_back(i);

        auto min_ind = *std::min_element(peak_inds.begin(), peak_inds.end());
        if (min_ind > 2)
            peak_inds.emplace_back(0);
        auto max_ind = *std::max_element(peak_inds.begin(), peak_inds.end());
        if (max_ind < peak.size() - 3)
            peak_inds.emplace_back(peak.size() - 1);

        std::sort(peak_inds.begin(), peak_inds.end());

        return peak_inds;
    }

    template <class DB>
    static void run(DB& db)
    {
        auto tmp = Pipe<
            Normalization<cv::NORM_MINMAX, int16_t>
          , improc::Blur<3>
          , improc::SharpenFilter<double_(50.0)>
          , improc::MedianBlur<3>
          , Normalization<cv::NORM_MINMAX, uint8_t>
          , improc::Binarization<CV_THRESH_OTSU | CV_THRESH_BINARY>
        //    Normalization<cv::NORM_MINMAX, uint8_t>
        //  , MedianBlur<3>
        //  , SharpenFilter<double_(3.0)>
        //  , Binarization<cv::THRESH_BINARY | cv::THRESH_OTSU>
        >::run(db.image.clone());

        // cv::imshow("tmp", tmp);

        std::vector<float> histx, histy;
        {
            cv::Mat horz;
            cv::reduce(tmp, horz, 0, CV_REDUCE_AVG, CV_32F);
            std::copy(
                horz.begin<float>()
              , horz.end<float>()
              , std::back_inserter(histx)
            );

            // {
            //     std::ofstream os("histx.txt");
            //     os << histx[0];
            //     for (auto i = 1ul; i < histx.size(); ++i)
            //         os << "\t" << histx[i];
            //     os << '\n';
            // }

            cv::Mat vert;
            cv::reduce(tmp, vert, 1, CV_REDUCE_AVG, CV_32F);
            std::copy(
                vert.begin<float>()
              , vert.end<float>()
              , std::back_inserter(histy)
            );

            // {
            //     std::ofstream os("histy.txt");
            //     os << histy[0];
            //     for (auto i = 1ul; i < histy.size(); ++i)
            //         os << "\t" << histy[i];
            //     os << '\n';
            // }
        }

        double radius = std::round(db.scale);
        auto&& xpeaks = find_peaks(histx, radius);
        auto&& ypeaks = find_peaks(histy, radius);

        // Fix missing grid line
        for (int32_t i = 1; i != xpeaks.size(); ++i)
            if ((xpeaks[i] - xpeaks[i - 1]) >= 2 * radius)
                xpeaks.insert(xpeaks.begin() + i, xpeaks[i - 1] + radius);
        for (int32_t i = 1; i != ypeaks.size(); ++i)
            if ((ypeaks[i] - ypeaks[i - 1]) >= 2 * radius)
                ypeaks.insert(ypeaks.begin() + i, ypeaks[i - 1] + radius);

        // auto c = cmax(tmp);
        // std::cerr << "gridx.size = " << xpeaks.size() << '\n';
        // for (auto& x: xpeaks)
        //     cv::line(
        //         tmp
        //       , cv::Point(x, 0)
        //       , cv::Point(x, tmp.rows)
        //       , cv::Scalar(c)
        //     );
        // std::cerr << "gridy.size = " << ypeaks.size() << '\n';
        // for (auto& y: ypeaks)
        //     cv::line(
        //         tmp
        //       , cv::Point(0, y)
        //       , cv::Point(tmp.cols, y)
        //       , cv::Scalar(c)
        //     );
        // cv::imshow("2", tmp);

        std::vector<cv::Rect> rois;
        for (auto j = 1ul; j != ypeaks.size(); ++j)
        {
            auto y0 = ypeaks[j - 1];
            auto y1 = ypeaks[j];

            // auto maxval = *std::max_element(
            //     histy.begin() + y0
            //   , histy.begin() + y1
            // );
            // auto lthres = std::floor((maxval + histy[y0]) * 0.5);
            // while (histy[y0] < lthres and y0 + 1 < y1 and y1 - y0 > radius)
            //     y0++;
            // auto rthres = std::floor((maxval + histy[y1]) * 0.5);
            // while (histy[y1] < rthres and y0 + 1 < y1 and y1 - y0 > radius)
            //     y1--;

            for (auto i = 1ul; i != xpeaks.size(); ++i)  
            {
                auto x0 = xpeaks[i - 1];
                auto x1 = xpeaks[i];

                // auto maxval = *std::max_element(
                //     histx.begin() + x0
                //   , histx.begin() + x1
                // );
                // auto lthres = std::floor((maxval + histx[x0]) * 0.5);
                // while (histx[x0] < lthres and x0 + 1 < x1 and x1 - x0 > radius)
                //     x0++;
                // auto rthres = std::floor((maxval + histx[x1]) * 0.5);
                // while (histx[x1] < rthres and x0 + 1 < x1 and x1 - x0 > radius)
                //     x1--;
                
                rois.emplace_back(cv::Point(x0, y0), cv::Point(x1, y1));
            }
        }
        db.rois = std::move(rois);
        db.ncols = xpeaks.size() - 1;
        db.nrows = ypeaks.size() - 1;
    }
};

// void print(cv::Mat& view)
// {
//     for (auto itr = view.begin<float>(); itr != view.end<float>(); ++itr)
//         std::cerr << *itr << '\n';
// }

/// Summary
/** This module is used to summerize the region of extracted probe cell
 *  based on the choice of policy. Currently it is not used in this pipeline.
 */
template <class POLICY>
class Summary
{
  public:
    using ValueType = typename POLICY::ValueType;

    template <class DB>
    static void run(DB& db)
    {
        std::vector<ValueType> summary;
        auto& image = db.image;
        auto& rois = db.rois;
        for (auto& roi: rois)
        {
            summary.emplace_back(
                POLICY::calc(
                    image
                  , roi
                  , db.lowest
                  , db.highest
                )
            );
        }
        db.data = std::move(summary);
    }
};

/// Buffer
/** This module is used to contain the necessary parameters for all components */
class Buffer
    : public RotationAngleEstimation::Parameters
    , public FeatureSizeDetection2::Parameters
{
  public:
    cv::Mat image;
    cv::Mat templ;
    uint16_t ncols;
    uint16_t nrows;
    std::vector<cv::Rect> rois;
    double highest;
    double lowest;
    std::vector<Intensity> data;

    std::string type;
    std::string barcode;
    cv::Mat mean;
    cv::Mat stddev;
    cv::Mat pixels;

    Buffer(void) = default;
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = default;
};

/// CEN file writer
/** This module is used to export the pixel intensities into CEN file format
 *  Currently, this CEN file has not yet fully implemented. We will complete
 *  this part as soon as possible
 */
// template <class DB>
// void save_as_cenfile(const boost::filesystem::path& path, DB&& db)
// {
//     namespace hdf = cpt::format::hdf5;
// 
//     std::cerr << "write image data into cenfile" << '\n';
// 
//     // constants
//     // uint16_t feature_cols = db.ncols;
//     // uint16_t feature_rows = db.nrows;
//     uint16_t num_channels = 1;
// 
//     // [hdf5] /
//     std::string fname = path.string();
//     hdf::File file(fname, H5F_ACC_TRUNC);
//     auto group = file.open_group("/");
//     group.create_attribute("magic", uint8_t(67));
//     group.create_attribute("version", uint8_t(1));
//     group.create_attribute("status", std::string("initialized"));
// 
//     // [hdf5] /array
//     auto array = group.create_group("array");
//     array.create_attribute("type", db.type);
//     array.create_attribute("barcode", db.barcode);
//     array.create_attribute("feature-cols", db.ncols);
//     array.create_attribute("feature-rows", db.nrows);
//     array.create_attribute("feature-width", db.scale);
//     array.create_attribute("feature-height", db.scale);
//     array.create_attribute("num-channels", num_channels);
// 
//     // [hdf5] data
//     std::vector<Intensity> buffer;
//     size_t total = db.nrows * db.ncols; 
//     auto& mean = db.mean;
//     auto& stddev = db.stddev;
//     auto& pixels = db.pixels;
//     for (size_t i = 0; i != total; ++i)
//     {
//         buffer.emplace_back(
//             mean.template at<float>(i)
//           , stddev.template at<float>(i)
//           , pixels.template at<uint16_t>(i)
//         );
//     }
// 
//     auto channel = array.create_group("channel-" + std::to_string(0));
//     channel.create_dataset(
//         "data"
//       , buffer
//       , { buffer.size() }
//     );
//     
//     // [hdf5] mask
//     std::vector<uint32_t> mask { 1, 2, 3, 4, 5 };
//     channel.create_dataset(
//         "mask"
//       , mask
//       , { mask.size() }
//     );
// 
//     // [hdf5] outlier
//     std::vector<uint32_t> outliers { 6, 7, 8, 9, 0 };
//     channel.create_dataset(
//         "outlier"
//       , outliers
//       , { outliers.size() }
//     );
// };

/// Trimmed mean method
/** This module is used to calculate the trimmed mean of probe cell
 *  given the region obtained from the ImageGridding component
 *  It will output a tuple of the trimmed mean, trimmed standard deviation,
 *  and the number of pixel intensities for summarization.
 */
class TrimmedMean
{
  public:
    using ValueType = std::tuple<float, float, uint16_t>;

  private:
    double lowest_, highest_;

  public:
    template <class DB>
    TrimmedMean(DB& db)
      : lowest_(db.lowest)
      , highest_(db.highest)
    {}

    auto operator()(cv::Mat& image, cv::Rect roi)
    {
        if (roi.width > 3)
        {
            roi.x += 1;
            roi.width -= 2;
        }
        if (roi.height > 3)
        {
            roi.y += 1;
            roi.height -= 2;
        }

        auto view = image(roi);

        std::sort(view.begin<float>(), view.end<float>());
        auto l = static_cast<int>(lowest_  * view.total());
        auto h = static_cast<int>(highest_ * view.total());

        float mean = std::accumulate(
            view.begin<float>() + l
          , view.begin<float>() + h
          , 0.0
        );
        mean /= (h - l);

        float stddev = std::accumulate(
            view.begin<float>() + l
          , view.begin<float>() + h
          , 0.0
          , [mean](const auto& init, const auto& elem)
            { return init + (elem - mean) * (elem - mean); }
        );
        stddev = std::sqrt(stddev / (h - l - 1));
        
        return ValueType(mean, stddev, h - l);
    }
};

/// Summary (version 2)
/** This module is used to summerize the probe cell based on the choice of policy.
 */
template <class POLICY>
class Summary2
{
  public:
    template <class DB>
    static void run(DB& db)
    {
        auto& image = db.image;
        auto& rois = db.rois;
        POLICY policy(db);
        
        cv::Mat mean  (cv::Size(0, 0), CV_32FC1);
        cv::Mat stddev(cv::Size(0, 0), CV_32FC1);
        cv::Mat pixels(cv::Size(0, 0), CV_16UC1);
        for (auto& roi: rois)
        {
            auto res = policy(image, roi);
            mean.push_back(std::get<0>(res));
            stddev.push_back(std::get<1>(res));
            pixels.push_back(std::get<2>(res));
        }
        db.mean   = std::move(mean.reshape(1, db.nrows));
        db.stddev = std::move(stddev.reshape(1, db.nrows));
        db.pixels = std::move(pixels.reshape(1, db.nrows));
    }
};


} // namespace improce


int main(int argc, char* argv[])
{
    namespace im = improc;
    namespace bfs = boost::filesystem;
    namespace bpo = boost::program_options;
    namespace bpt = boost::property_tree;
    
    /**
     *  Configuration
     */

    cv::Mat_<uint8_t> marker; ///< the predefined pattern for detecting the feature size
    bfs::path outdir;
    bfs::path marker_style;
    std::string prefix;
    std::vector<bfs::path> input_paths;
    std::set<std::string> export_opts{ "cen" };
    std::vector<double> theta{ -5.0, 5.0, 0.01 };
    double min_feat_size = -1.0;
    std::vector<double> percentiles{ 0.0, 1.0 };
    int32_t default_coi = 0; // default color of interest = red channel
    std::string wname0 = "source image";
    std::string wname1 = "gridding result";
    std::string wname2 = "extracted signals";

    bpo::options_description common("Common options");
    common.add_options()
    
      ( "help,h", "show this help messages" )
    //( "verbose,v", "show detail messages" )
      ( "gui", "display images in graphical user interface "
               "(X window or MobaXterm client needed" )
      ;
    
    bpo::options_description ioconfig("input/output configuration");
    ioconfig.add_options()

      ( "marker,m", bpo::value(&marker_style)->required()
      , "file path to the marker style description" )
      ( "images,i", bpo::value(&input_paths)->multitoken()->required()
      , "file path to the microarray images" )
      ( "outdir,o", bpo::value(&outdir)->default_value(".")
      , "path to the output directory" )
      ( "prefix", bpo::value(&prefix)->default_value("")
      , "prefix of output files" )
      ( "export,e"
      , bpo::value<std::vector<std::string>>()
        ->multitoken()->zero_tokens()->notifier(
        [&export_opts](const auto& opts)
        {
            export_opts.clear();
            export_opts.insert(opts.begin(), opts.end());
        })
      , "configuration of export formats. Possible values could be "
        "\"cen\", \"txt\" and \"img\"" )
      ;
    
    bpo::options_description process("Options of image processing");
    process.add_options()

      ( "angle"
      , bpo::value(&theta)->multitoken()->notifier(
        [](const auto& opt)
        {
            if (opt.size() != 3)
                throw std::invalid_argument("Invalid angle sweeping range description");
        })
      , "Calibrated angle sweeping range" )
      ( "feat-size"
      , bpo::value(&min_feat_size)
      , "minimum feature size" )
      ( "trim"
      , bpo::value(&percentiles)->multitoken()->notifier(
        [](const auto& opt)
        {
            if (opt.size() != 2 or
                opt[0] < 0.0 or opt[0] > 1.0 or
                opt[1] < 0.0 or opt[1] > 1.0)
                throw std::invalid_argument("Invalid trimming range");
        })
      , "define the bounds of the percentile for dropping extreme values. "
        "(default = [ 0.0 , 1.0 ])" )
      ( "auto-crop"
      , "export the region of interest based on the maximum "
        "spanning area of markers automatically" )
      ( "marker-qc"
      , "extract the signals of marker only" )
      // ( "channel", bpo::value(&default_coi)->default_value(0)->notifier(
      //              [](const auto& opt)
      //              {
      //                  if (opt < 0 or opt > 2)
      //                      throw std::invalid_argument("channel is either 0, 1 or 2");
      //              })
      // , "select the channel of interest. "
      //   "The value is either 0, 1 or 2 for a full-color image "
      //   "and 0 only for a grayscale image" )
      ;

    //TODO Refactor this part
    bpo::options_description desc(
        "\nMicroarray Image Data Extraction Pipeline\n\n"
        "This program allows the user to extract the signal intensities from \n"
        "microarray images. The process includes image auto adjustment, image \n"
        "gridding, and intensity extraction as files in CEN or plain-text format.\n"
        "The usage is shown as follows:"
    );
    desc.add(common).add(ioconfig).add(process);
    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
    if (argc == 1 or vm.count("help"))
    {
        desc.print(std::cerr);
        std::cerr << '\n';
        std::cerr <<
        "Examples:\n"
        "1. Input N images and output the corresponding results to an \"outputs\" directory. "
           "This will export a CEN file in default.\n\n"
           "    " << argv[0] << " -i <image1> ... <imageN> -o outputs\n\n"
        "2. Input images by using wildcard charactor.\n\n"
           "    " << argv[0] << " -i data/*.bmp -o outputs\n\n"
        "3. Display the images sequentially in graphical user interface. "
           "The grayscale boxes denote the extracted regions.\n\n"
           "    " << argv[0] << " -i <image1> ... <imageN> -o outputs --gui\n\n"
        "4. Export the quantities measured from the intensity values of pixels within each feature "
           "execpt for the top 5\% of the highest values and the bottom 50\% of the lowest values.\n\n"
           "    " << argv[0] << " -i <image1> ... <imageN> -o outputs -e cen txt img"
                                " --trim 0.5 0.95\n\n"
        "5. Export the image based on the maximum spanning region of markers automatically\n\n"
           "    " << argv[0] << " -i <image1> ... <imageN> -o outputs --auto-crop\n\n"
        ;
        return 1;
    }
    bpo::notify(vm);

    if (!bfs::exists(outdir))
    {
        bfs::create_directory(outdir);
    }
    if (vm.count("gui"))
    {
        /**
         *  setting the graphical interface
         */

        cv::namedWindow(wname0, CV_WINDOW_NORMAL);
        cv::moveWindow(wname0, 40, 50);
        cv::resizeWindow(wname0, 960, 720);

        cv::namedWindow(wname1, CV_WINDOW_NORMAL);
        cv::moveWindow(wname1, 60, 70);
        cv::resizeWindow(wname1, 960, 720); 

        cv::namedWindow(wname2, CV_WINDOW_NORMAL);
        cv::moveWindow(wname2, 80, 90);
        cv::resizeWindow(wname2, 960, 720); 
    }

    /**
     *  Check the avalibility of input files
     */
    {
        bool terminate = false;

        std::cerr << "Input files:\n";
        for (auto& path: input_paths)
        {
            if (!bfs::exists(path))
            {
                std::cerr << path.string() << " not found\n";
                terminate = true;
            }
            else
            {
                std::cerr << "+ " << path.string() << '\n';
            }
        }
        std::cerr << "Marker style:\n";
        if (!bfs::exists(marker_style))
        {
            std::cerr << marker_style.string() << " not found\n";
            terminate = true;
        }
        else
        {
            std::cerr << "+ " << marker_style.string() << '\n';
        }
        if (terminate)
        {
            throw std::runtime_error("Some input files are not found. Abort!");
        }
    }

    /**
     *  Display the path to output directory
     */
    std::cerr << "Output path:\n"
              << "+ " << outdir.string() << '\n';

    auto cwd = bfs::path(argv[0]).parent_path();
    auto templ = improc::imread((cwd / "marker1.tif").string());
    auto json = cpt::format::make_json();
    auto jimages = json.create_list("images");
    bool json_marker_created = true;

    /**
     *  Load the description of marker style
     */
    {
        std::ifstream is(marker_style.string());
        char c;
        marker = decltype(marker)::zeros(10, 10);
        for (auto i = 0; is.good() and !is.eof() and i != 100; )
        {
            c = is.get();
            if (c == 'X' or c == '.')
                marker(i++) = (c == 'X')? 255: 0;
        }
        is.close();
    }

    /**
     *  Create an empty log file for marker-qc
     */
    auto path_to_marker_qc = outdir;
    path_to_marker_qc /= prefix + "marker_qc.txt";
    if (vm.count("marker-qc"))
    {
        std::ofstream os(path_to_marker_qc.string());
        os.close();
    }

    /**
     *  Process for all input images
     */
    for (auto& input_path: input_paths)
    {
        std::cerr << input_path.string() << '\n';
        // set output path
        auto output1 = outdir; output1 /= prefix + input_path.stem().string() + ".grid.tif";
        auto output2 = outdir; output2 /= prefix + input_path.stem().string() + ".txt";
        auto output3 = outdir; output3 /= prefix + input_path.stem().string() + ".cen";

        // load image
        auto image = im::imread(input_path.string());

        // show image
        if (vm.count("gui"))
        {
            imshow(wname0, image);
            // cv::updateWindow(wname0);
        }

        // prepare a pipeline buffer
        im::Buffer db;
        db.templ   = templ;
        db.lowest  = percentiles[0];
        db.highest = percentiles[1];
        db.type    = "Centrillion-AM1";
        int channel = image.channels() - default_coi - 1;
        if (channel < 0 and channel > 2)
        {
            throw std::invalid_argument("invalid channel number selection");
        }
        db.tmin = theta[0] + 90.0;
        db.tmax = theta[1] + 90.0;
        db.tres = theta[2];
        db.scale = min_feat_size;

        // estimate calibration parameters
        cv::extractChannel(image, db.image, channel);
        im::Pipe<
            im::RotationAngleEstimation
          , im::RotationAngleCalibration
          , im::FeatureSizeDetection2
        >::run(db);

        // extract signal intensities
        cv::extractChannel(image, db.image, channel);
        im::ConvertTo<float, false>::run(db.image);
        im::Pipe<
            im::RotationAngleCalibration
          , im::ImageGridding
          , im::Summary2<im::TrimmedMean>
        >::run(db);

        // draw gridding result
        im::imrotate(image, db.angle);
        const auto cmax = im::cmax(image) / 2;
        for (auto roi: db.rois)
        {
            roi.width += 1;
            roi.height += 1;
            cv::rectangle(image, roi, cv::Scalar(cmax, cmax, cmax));
        }

        // export gridding result
        if (export_opts.count("img"))
        {
            cv::imwrite(output1.string(), image);
        }

        // show gridding result
        if (vm.count("gui"))
        {
            cv::imshow(wname1, image);
            // cv::updateWindow(wname1);
        }
        
        // auto detect the region of interest by marker positions
        if (vm.count("auto-crop"))
        {
            jimages.push_back(output2.string());

            cv::Mat_<uint8_t> bw = im::Pipe<
                im::Normalization<cv::NORM_MINMAX, uint8_t>
            >::run(db.mean.clone());

            // cv::Mat unused;
            cv::threshold(bw, bw, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);

            // matching template
            cv::Mat_<float> scores(
                bw.cols - marker.cols + 1
              , bw.rows - marker.rows + 1
            );
            cv::matchTemplate(bw, marker, scores, CV_TM_CCORR_NORMED);
            
            const int32_t texp = 0;
            const int32_t bexp = 0;
            const int32_t lexp = 0;
            const int32_t rexp = 0;

            std::vector<cv::Point> offsets;
            for (auto r = 0; r != scores.rows; ++r)
                for (auto c = 0; c != scores.cols; ++c)
                    if (scores(r, c) >= 0.85)
                    // if (scores(r, c) > 0.88)
                        offsets.emplace_back(c, r);

            if (!offsets.empty())
            {
                std::cerr << offsets.size() << " markers are detected\n";

                if (vm.count("marker-qc"))
                {
                    double total = 0;
                    double mean = 0;
                    double stddev = 0;
                    double prev_mean = 0;
                    
                    for (auto& offset: offsets)
                    {
                        cv::Mat roi = db.mean(
                            cv::Rect(
                                offset.x
                              , offset.y
                              , marker.cols
                              , marker.rows
                            )
                        );

                        for (auto i = 0; i != marker.total(); ++i)
                        {
                            if (marker(i) < 128)
                                continue;

                            auto&& value = roi.at<float>(i);
                            prev_mean = mean;
                            total += 1;
                            mean += (value - mean) / total;
                            stddev += (value - mean) * (value - prev_mean);
                        }
                    }
                    stddev = std::sqrt(stddev / (total - 1));

                    std::ofstream os(
                        path_to_marker_qc.string()
                      , std::ofstream::out
                      | std::ofstream::app
                    );
                    os << std::fixed;
                    os << input_path.stem().string() << '\t'
                       << std::setprecision(2) << mean << '\t'
                       << std::setprecision(2) << stddev << '\t'
                       << total << '\t'
                       << std::setprecision(2) << stddev / mean * 100 << '\n';
                    os.close();
                }

                auto roi = cv::boundingRect(offsets);
                roi.x -= lexp;
                roi.y -= rexp;
                roi.width  += marker.cols - 1 + lexp + rexp;
                roi.height += marker.rows - 1 + texp + bexp;

                if (!json_marker_created)
                {
                    int32_t dim = std::ceil(std::sqrt(input_paths.size()));
                    json_marker_created = true;
                    json.add("cols", dim);
                    json.add("rows", dim);
                    json.add("width" , roi.width);
                    json.add("height", roi.height);
                    auto jlist = json.create_list("markers");

                    std::vector<cv::Point> spots;
                    cv::findNonZero(marker, spots);
                    for (auto& offset: offsets)
                    {
                        for (auto& spot: spots) 
                        {
                            jlist.push_back(offset.x - roi.x + spot.x);
                            jlist.push_back(offset.y - roi.y + spot.y);
                        }
                    }
                    std::vector<int> exp{ texp, bexp, lexp, rexp };
                    json.add("expanse", exp);
                }

                std::cerr << roi << '\n';
                db.mean = db.mean(roi);
                db.stddev = db.stddev(roi);
                db.pixels = db.pixels(roi);
            }
            else
            {
                std::cerr << "No markers are detected\n";
            }
        }

        // save as cenfile
        // if (export_opts.count("cen"))
        //     im::save_as_cenfile(output3, db);

        // save as plain-text (mean only)
        if (export_opts.count("txt"))
        {
            std::ofstream os(output2.string());
            for (auto j = 0ul; j != db.mean.rows; ++j)
            {
                os << db.mean.at<float>(j, 0);
                // os << db.stddev.at<float>(j, 0) / db.mean.at<float>(j, 0) * 100;
                for (auto i = 1ul; i != db.mean.cols; ++i)
                    os << '\t' << db.mean.at<float>(j, i);
                    // os << '\t' << db.stddev.at<float>(j, i) / db.mean.at<float>(j, i) * 100;
                os << '\n';
            }
        }

        if (vm.count("gui"))
        {
            auto tmp = im::Normalization<
                cv::NORM_MINMAX
              , float
            >::run(db.mean.clone());
            cv::imshow(wname2, tmp);
            cv::waitKey(0);
        }
    }

    if (vm.count("auto-crop"))
    {
        auto output = outdir;
        output /= prefix + ".json";
        std::ofstream os(output.string());
        json.dump(os, true);
    }

    return 0;
}
