#include "pipeline.hpp"
#include "filters.hpp"
#include "utils.hpp"
#include "intensity.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
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

//TODO Robust projection method
//TODO Adaptive gridding method
//TODO Control CV image segmentation
//TODO Faster hough transform
//TODO CEN file reporter
//TODO General image data type convertor & normalizer
//TODO Uniformality calibration by using linear regression
//TODO Checkpoint verbose
//TODO Pipeline option configurations
//TODO Better image stitching algorithm
//TODO Gridline stitching
//TODO EM algorithm for feature quality control
//TODO Simulated image data:
//       : large scale
//       , gaussian noise
//       , halo effect
//       , scratch
//       , saturation
//       , deletion
//       , bubble/donut shape
//       , square feature shape
//       , circle feature shape
//       , ellipse feature shape
//       , polygon feature shape
//       , nonlinear relationship between stddev-mean

namespace improc {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class RotationCalibration
{
  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        auto& image = buf.image;
        cv::Point2f center(image.cols >> 1, image.rows >> 1);
        auto mat = cv::getRotationMatrix2D(center, -buf.angle, 1.0);
        cv::warpAffine(image, image, mat, image.size());
    }
};

class RotationEstimation
{
  using FLOAT = double;

  public:
    struct Parameters
    {
        FLOAT nbins;
        FLOAT lower;
        FLOAT upper;
    };
 
  private:
    struct UnitVector
    {
        FLOAT theta;
        FLOAT cos_theta;
        FLOAT sin_theta;

        UnitVector(FLOAT theta)
          : theta(theta)
          , cos_theta(std::cos(theta * CV_PI / 180.0))
          , sin_theta(std::sin(theta * CV_PI / 180.0))
        {}

        FLOAT rho(const int32_t x, const int32_t y) const
        {
            return x * cos_theta + y * sin_theta;
        }
    };

    static FLOAT rho(const int32_t x, const int32_t y, const FLOAT theta)
    {
        return x * std::cos(theta * CV_PI / 180.0)
             + y * std::sin(theta * CV_PI / 180.0);
    }

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        auto image = buf.image.clone();
        const auto cols = image.cols;
        const auto rows = image.rows;
        const auto tmin = buf.lower;
        const auto tmax = buf.upper;
        const auto tmid = std::atan2(rows, cols) * 180.0 / CV_PI;
        const auto rmin = std::floor(
            (tmax > 90.0)? rho(cols, 0, tmax): 0.0
        );
        const FLOAT rmax = std::ceil(
            (tmax < tmid)? rho(cols, rows, tmax)
          : (tmin < tmid)? std::sqrt(rows * rows + cols * cols)
          : (tmin < 90.0)? rho(cols, rows, tmin)
          :                rho(0   , rows, tmin)
        );
        const auto tstep = (tmax - tmin) / buf.nbins;

        std::vector<UnitVector> unitvecs;
        for (auto theta = tmin; theta < tmax; theta += tstep)
            unitvecs.emplace_back(theta);

        cv::Mat unused;
        cv::normalize(image, image, 0.0, 255.0, cv::NORM_MINMAX, CV_8U);
        auto thres = cv::threshold(image, unused, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
        cv::Canny(image, image, thres * 0.9, cv::saturate_cast<uint8_t>(thres * 1.0));

        // auto wname = std::to_string(__LINE__);
        // cv::namedWindow(wname, cv::WINDOW_NORMAL);
        // cv::resizeWindow(wname, 960, 720);
        // cv::imshow(wname, image);
        // cv::waitKey(0);

        cv::Mat_<FLOAT> hist = cv::Mat_<FLOAT>::zeros(unitvecs.size(), rmax - rmin + 1);
        for (auto r = 0; r != image.rows; ++r)
            for (auto c = 0; c != image.cols; ++c)
                if (image.template at<uint8_t>(r, c) > 127)
                    for (auto& u: unitvecs)
                    {
                        hist(
                            std::round((u.theta     - tmin) / tstep)
                          , std::round((u.rho(c, r) - rmin)        )
                        ) += 1.0;
                    }

        double val;
        cv::Point loc;
        cv::minMaxLoc(hist, nullptr, &val, nullptr, &loc);
        buf.angle = unitvecs[loc.y].theta - 90.0;

        std::cerr << "angle = " << buf.angle << '\n';

        // cv::normalize(hist, hist, 0.0, 1.0, cv::NORM_MINMAX, hist.depth());
        // cv::line(hist, cv::Point(0, loc.y), cv::Point(hist.cols-1, loc.y), cv::Scalar(1.0));
        // auto wname = std::to_string(__LINE__);
        // cv::namedWindow(wname, cv::WINDOW_NORMAL);
        // cv::resizeWindow(wname, 960, 720);
        // cv::imshow(wname, hist);
        // cv::waitKey(0);
    }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class Gridding
{
  using FLOAT = double;

  public:
    struct Parameters
    {
        bool adaptive;
    };

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        auto src = buf.image.clone();
        auto x = fit_sinewave<0,18>(src);
        auto y = fit_sinewave<1,18>(src);

        std::vector<cv::Rect> rois;
        for (auto j = 1; j != y.size(); ++j)
        {
            const auto y1 = std::round(y[j - 1]);
            const auto y2 = std::round(y[j]);
            for (auto i = 1; i != x.size(); ++i)
            {
                const auto x1 = std::round(x[i - 1]);
                const auto x2 = std::round(x[i]);
                rois.emplace_back(
                    cv::Point(x1, y1)
                  , cv::Point(x2, y2)
                );
            }
        }

        buf.rois  = std::move(rois);
        buf.nrows = y.size() - 1;
        buf.ncols = x.size() - 1;
    }

  private:
    template <int32_t dim, int32_t lg2nfft>
    static auto fit_sinewave(const cv::Mat_<FLOAT>& src)
    {
        // 1D projection
        cv::Mat_<FLOAT> data;
        cv::reduce(src, data, dim, CV_REDUCE_AVG, data.depth());
        if (dim == 1)
            cv::transpose(data, data);

        // Moving average
        cv::Mat_<FLOAT> ac = cv::Mat_<FLOAT>::zeros(1, 1 << lg2nfft);
        for (auto i = 2; i != data.total(); ++i)
            ac(i - 2) = ( 2 * data(i) - data(i - 1) - data(i - 2) ) / 3;
        
        // Frequency detection
        cv::Mat_<cv::Point_<FLOAT>> ft;
        cv::dft(ac, ft, cv::DFT_COMPLEX_OUTPUT);
        FLOAT max = 0.0;
        FLOAT loc = 0.0;
        auto last = ft.total() >> 1;
        for (auto i = 0; i != last; ++i)
        {
            auto val = cv::norm(ft(i));
            if (max < val)
            {
                max = val;
                loc = i;
            }
        }
        const double freq = loc / static_cast<FLOAT>(ft.total());
        std::cerr << "intvl = " << 1.0 / freq << '\n';

        // Phase estimation
        cv::Mat_<FLOAT> Phi, w;
        const auto v = 2.0 * CV_PI * freq;
        for (auto i = 0; i != data.total(); ++i)
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
        std::cerr << "phase = " << phase << '\n';
        
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
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class Segmentation
{
    using FLOAT = double;

  public:
    struct Parameters
    {
        // std::vector<uint32_t> padding;
    };

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        for (auto& roi: buf.rois)
        {
            if (roi.width > 5)
            {
                roi.x += 2;
                roi.width -= 4;
            }
            if (roi.height > 5)
            {
                roi.y += 2;
                roi.height -= 4;
            }
        }
    }
};


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class ROIDetection
{
  public:
    struct Parameters
    {
        cv::Mat_<uint8_t> marker;
    };

  public:
    template <class BUFFER>
    static void run(BUFFER&& buf)
    {
        cv::Mat_<uint8_t> bw = Pipe<
            Normalization<cv::NORM_MINMAX, uint8_t>
        >::run(buf.mean.clone());

        // cv::Mat unused;
        cv::threshold(bw, bw, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);

        // matching template
        cv::Mat_<float> scores(
            bw.cols - buf.marker.cols + 1
          , bw.rows - buf.marker.rows + 1
        );
        cv::matchTemplate(bw, buf.marker, scores, CV_TM_CCORR_NORMED);
        
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

            auto roi = cv::boundingRect(offsets);
            roi.x -= lexp;
            roi.y -= rexp;
            roi.width  += buf.marker.cols - 1 + lexp + rexp;
            roi.height += buf.marker.rows - 1 + texp + bexp;

            std::cerr << roi << '\n';
            buf.mean = buf.mean(roi);
            buf.stddev = buf.stddev(roi);
            buf.pixels = buf.pixels(roi);
        }
        else
        {
            std::cerr << "No markers are detected\n";
        }
    }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/// Buffer
/** This module is used to contain the necessary parameters for all components */
class Buffer
    : public RotationEstimation::Parameters
    , public FeatureSizeDetection2::Parameters
    , public Gridding::Parameters
{
  public:
    cv::Mat image;
    cv::Mat templ;
    cv::Mat_<uint8_t> marker;

    double angle;
    std::vector<cv::Rect> rois;
    uint16_t ncols;
    uint16_t nrows;

    double highest;
    double lowest;
    std::vector<Intensity> data;
    std::string type;
    std::string barcode;

    cv::Mat mean;
    cv::Mat stddev;
    cv::Mat pixels;

    Buffer(void) = default;
    Buffer(const Buffer&) = default;
    Buffer(Buffer&&) = default;
};

} // namespace improce

struct PathChecker
{
    bool good;

    PathChecker(void)
      : good(true)
    {}
    void operator()(const boost::filesystem::path& path)
    {
        if (!boost::filesystem::exists(path))
        {
            std::cerr << path << " not found!\n";
            good = false;
        }
        else
        {
            std::cerr << path << " found\n";
        }
    }
    void verify(void) const
    {
        if (!good)
            abort();
    }
};

int main(int argc, char* argv[])
{
    namespace im = improc;
    namespace bfs = boost::filesystem;
    namespace bpo = boost::program_options;
    namespace bpt = boost::property_tree;

    //
    // parse command line
    
    bfs::path path_to_input;
    bfs::path path_to_output;
    bfs::path path_to_chip_list;
    bfs::path path_to_pipe_opts;
    bfs::path workdir = argv[0];
    workdir.remove_filename();
    int32_t num_threads;

    PathChecker path_checker;

    bpo::options_description common("Common options");
    common.add_options()
      ( "help,h", "show this help messages" )
      ;
    bpo::options_description ioconfig("input/output configuration");
    ioconfig.add_options()
      ( "input,i", bpo::value(&path_to_input)
                   ->required()
                   ->notifier(path_checker)
      , "file path to input json file" )
      ( "output,o", bpo::value(&path_to_output)
                    ->required()
      , "file path to output json file" )
      ( "chip-list,c", bpo::value(&path_to_chip_list)
                       ->default_value(workdir / "chip_list.json")
                       ->notifier(path_checker)
      , "file path to chip list" )
      ( "pipe-opts,p", bpo::value(&path_to_pipe_opts)
                       ->default_value(workdir / "pipe_opts.json")
                       ->notifier(path_checker)
      , "file path to pipeline options" )
      ( "num-threads,j", bpo::value(&num_threads)->default_value(0)
      , "number of threads used by OpenCV" )
      ;
    bpo::options_description desc(
        "\nMicroarray Image Data Extraction Pipeline\n\n"
        "This program allows the user to extract the signal intensities from \n"
        "microarray images. The process includes image auto adjustment, image \n"
        "gridding, and intensity extraction as files in CEN or plain-text format.\n"
        "The usage is shown as follows:"
    );
    desc.add(common).add(ioconfig);
    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
    if (argc == 1 or vm.count("help"))
    {
        desc.print(std::cerr);
        return 1;
    }
    bpo::notify(vm);
    path_checker(path_to_chip_list);
    path_checker(path_to_pipe_opts);
    path_checker.verify();

    //
    // configure options and parameters
    // 
    cv::setNumThreads(num_threads);
    auto input_json = cpt::format::read_json(path_to_input.string());
    auto input_dir = input_json.get<bfs::path>("input_dir");
    auto output_dir = input_json.get<bfs::path>("output_dir");
    auto output_json = cpt::format::make_json();
    path_checker(input_dir);
    path_checker(output_dir);

    //
    // load array type and marker description file
    //  
    auto chip_list = cpt::format::read_json(path_to_chip_list.string());
    auto chip_found = false;
    auto array_type = input_json.get<std::string>("type");
    uint32_t reading_rows;
    uint32_t reading_cols;
    uint32_t total_images;
    bfs::path path_to_marker;
    for (auto& item: chip_list.get_list("array"))
    {
        auto& chip = item.second;
        if (chip.get<std::string>("type") == array_type)
        {
            chip_found = true; 
            path_to_marker = chip.get<std::string>("marker");
            reading_rows = chip.get<uint32_t>("reading_rows");
            reading_cols = chip.get<uint32_t>("reading_cols");
            path_checker(path_to_marker);
            path_checker.verify();

            break;
        }
    }
    if (!chip_found)
    {
        std::cerr << "missing array type\n";
        abort();
    }
    total_images = reading_rows * reading_cols;

    std::vector<bfs::path> image_list;
    for (auto n = 1; n <= total_images; ++n)
    {
        char num[64] = {'\0'};
        sprintf(num, "%02d.tif", n);
        auto path_to_image = input_dir / std::string(num);
        path_checker(path_to_image);
        image_list.emplace_back(path_to_image);
    }
    path_checker.verify();
    
    //
    // load default options
    auto pipe_opts = cpt::format::read_json(path_to_pipe_opts.string());
    im::Buffer default_opts;
    {
        auto opts = pipe_opts.get_child("pipeline_options");
        default_opts.lower = opts.get<double>("rotation.lower") + 90.0;
        default_opts.upper = opts.get<double>("rotation.upper") + 90.0;
        default_opts.nbins = opts.get<double>("rotation.nbins");
        auto intvl = opts.get<std::string>("gridding.intvl");
        default_opts.scale = (intvl != "auto")
                           ? std::stod(intvl, nullptr): -1;
        default_opts.lowest  = opts.get<double>("summarization.trim_lower");
        default_opts.highest = opts.get<double>("summarization.trim_upper");
        default_opts.templ = improc::imread((workdir / "marker1.tif").string());

        std::ifstream is(path_to_marker.string());
        char c;
        default_opts.marker = decltype(default_opts.marker)::zeros(10, 10);
        for (auto i = 0; is.good() and !is.eof() and i != 100; )
        {
            c = is.get();
            if (c == 'X' or c == '.')
                default_opts.marker(i++) = (c == 'X')? 255: 0;
        }
        is.close();
    }

    // process for each image
    std::vector<cv::Mat_<float>> results;
    for (auto& path_to_image: image_list)
    {
        std::cerr << "\nprocess: " << path_to_image << '\n';
        auto buf = default_opts;
        auto raw_image = im::imread(path_to_image.string());

        auto channel = raw_image.channels() - 1;
        cv::extractChannel(raw_image, buf.image, channel);
        im::Pipe<
            im::RotationEstimation
          // , im::RotationCalibration
          // , im::FeatureSizeDetection2
        >::run(buf);

        im::ConvertTo<float, false>::run(buf.image);
        im::Pipe<
            im::RotationCalibration
          , im::Gridding
          , im::Summary2<im::TrimmedMean>
          , im::ROIDetection
        >::run(buf);

        // draw gridding result
        // auto tmp = buf.image.clone();
        // const auto cmax = im::cmax(tmp) / 2;
        // for (auto roi: buf.rois)
        // {
        //     roi.width += 1;
        //     roi.height += 1;
        //     cv::rectangle(tmp, roi, cv::Scalar(cmax, cmax, cmax));
        // }
        // cv::imshow("tmp", tmp);
        // cv::waitKey(0);
        
        // auto tmp = im::Normalization<
        //     cv::NORM_MINMAX
        //   , float
        // >::run(buf.mean.clone());
        // cv::imshow("tmp", tmp);
        // cv::waitKey(0);
        
        results.emplace_back(buf.mean.clone());
    }

    int32_t total_rows = 0;
    for (auto r = 0; r != reading_rows; ++r)
        total_rows += results[reading_cols * r].rows;

    int32_t total_cols = 0;
    for (auto c = 0; c != reading_cols; ++c)
        total_cols += results[c].cols;

    cv::Mat_<float> full(
        total_rows - 10 * (reading_rows - 1)
      , total_cols - 10 * (reading_cols - 1)
    );
    full = 0.0;
    int32_t row_offset = 0;
    for (auto r = 0; r != reading_rows; ++r)
    {
        int32_t col_offset = 0;
        for (auto c = 0; c != reading_cols; ++c)
        {
            auto& res = results[c + reading_cols * r];
            cv::Rect roi(
                col_offset
              , row_offset
              , res.cols
              , res.rows
            );
            res.copyTo(full(roi));

            col_offset += res.cols - 10;
            if (c == reading_cols - 1)
                row_offset += res.rows - 10;
        }
    }
   
    auto path_to_result_png = output_dir / "sample.bmp";
    output_json.add("img", path_to_result_png.string());
    output_json.add("rows", full.rows);
    output_json.add("cols", full.cols);

    std::ofstream os(path_to_output.string());
    output_json.dump(os);
    os.close();
    
    cv::Mat png;
    cv::normalize(full, png, 0.0, 255.0, cv::NORM_MINMAX, CV_8U);
    cv::Mat res(png.rows * 5, png.cols * 5, CV_8UC1);
    cv::resize(png, res, res.size(), 0, 0, cv::INTER_AREA);

    im::imwrite(path_to_result_png.string(), res);

    cv::imshow("tmp", res);
    cv::waitKey(0);

    // bfs::path path_to_marker_file;
    

    return 0;
}
