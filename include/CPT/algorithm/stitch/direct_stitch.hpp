#pragma once
#include <vector>
#include <CPT/improc/util.hpp>
#include <iostream>
namespace cpt{ namespace algorithm{ namespace stitch{

struct DirectStitch
{
    DirectStitch( int row, int col, int vert_off, int hor_off ) 
    : row_(row)
    , col_(col)
    , vert_off_( vert_off )
    , hor_off_(hor_off)
    {}
    cv::Mat operator() ( const std::vector<cv::Mat>& ims ) {
        if( ims.size() != row_ * col_) {
            throw std::logic_error(
                "the image number not match specified parameter"
            );

        }
        int height = 0;
        int width = 0;
        std::vector<int> hor_st_pos;
        std::vector<int> vert_st_pos;

        for ( int j = 0; j < col_; j ++ ) {
            hor_st_pos.push_back( width );
            auto& im = ims.at( j );
            width += im.cols;
            width -= hor_off_;
        }
        width += hor_off_;

        for ( int i = 0; i < row_; i ++ ) {
            vert_st_pos.push_back(height);
            auto& im = ims.at( col_* i );
            height += im.rows;
            height -= vert_off_;
        }
        height += vert_off_;

        cv::Mat res(cv::Size(width + 100, height + 100), ims.at(0).type());

        int imd = 0;
        for ( const auto& vp : vert_st_pos ) {
            for ( const auto& hp : hor_st_pos ) {
                auto& img = ims.at(imd);
                cpt::improc::info(img);
                if( imd == 1 || imd == 3 ) {
                    fill_region(
                        res, 
                        hp, vp + 30, 
                        img
                    );
                } else {
                    fill_region(
                        res, 
                        hp, vp, 
                        img
                    );
                }
                imd ++;
            }
        }
        return res;
    }
private:
    static void fill_region( 
        cv::Mat& dst, 
        int hp, int vp, 
        const cv::Mat& src
    ) {
        cv::Mat dst_roi = dst(cv::Rect(hp, vp, src.cols, src.rows));
        src.copyTo(dst_roi);
    }
    int row_;
    int col_;
    int vert_off_;
    int hor_off_;
};

}}}