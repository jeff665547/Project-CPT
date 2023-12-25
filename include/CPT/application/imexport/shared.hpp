#pragma once
#include <boost/python.hpp>
#include <opencv2/opencv.hpp>
#include "pipeline.hpp"
#include "filters.hpp"
#include "utils.hpp"
#include "intensity.hpp"
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

namespace cpt { namespace application { namespace imexport {
namespace bfs = boost::filesystem;

void improc_impl ( 
      bool                              gui
    , const bfs::path&                 marker_style
    , const std::vector<bfs::path> &   input_paths
    , const bfs::path&                 outdir
    , const std::string&                prefix
    , const std::vector<std::string>&   export_opts
    , const std::vector<double>     &   theta
    , const double&                     min_feat_size
    , const std::vector<double>     &   percentiles
    , bool                              auto_corp
)
{
    std::string wname0 = "source image";
    std::string wname1 = "gridding result";
    std::string wname2 = "extracted signals";
    if (!bfs::exists(outdir))
    {
        bfs::create_directory(outdir);
    }
    if (gui)
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

    // auto cwd = bfs::path(argv[0]).parent_path();
    // auto templ = improc::imread((cwd / "marker1.tif").string());
    // auto json = cpt::format::make_json();
    // auto jimages = json.create_list("images");
    // bool json_marker_created = true;

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
        if (export_opts.count("cen"))
            im::save_as_cenfile(output3, db);

        // save as plain-text (mean only)
        if (export_opts.count("txt"))
        {
            std::ofstream os(output2.string());
            for (auto j = 0ul; j != db.mean.rows; ++j)
            {
                // os << db.mean.at<float>(j, 0);
                os << db.stddev.at<float>(j, 0) / db.mean.at<float>(j, 0) * 100;
                for (auto i = 1ul; i != db.mean.cols; ++i)
                    // os << '\t' << db.mean.at<float>(j, i);
                    os << '\t' << db.stddev.at<float>(j, i) / db.mean.at<float>(j, i) * 100;
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

    // // const auto top_n = 12;
    // // std::partial_sort(
    // //     scores.begin<float>()
    // //   , scores.begin<float>() + top_n
    // //   , scores.end<float>()
    // //   , std::greater<float>()
    // // ); 
    // // for (auto i = 0l; i != top_n; ++i)
    // //     std::cerr << scores.at<float>(i) << '\n';
    
}

}}}
