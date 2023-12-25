#pragma once
#include <opencv2/opencv.hpp>
#include <CPT/application/intensities_dump/probe_id_mapper.hpp>
#include <iostream>
#include <CPT/application/intensities_dump/point.hpp>
#include <CPT/application/intensities_dump/utility.hpp>
#include <CPT/logger.hpp>
#include <Nucleona/language.hpp>
#include <CPT/view.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
template<
      class PROBE_ID
    , class MEAN
    , class STDDEV
    , class PIXELS
    , class X
    , class Y
    , class CV_VALUE
    , class DETAIL_RAW_VALUE
>
struct Cell
{
    PROBE_ID            probe_id            {}       ;
    MEAN                mean                {}       ;
    STDDEV              stddev              {}       ;
    PIXELS              pixels              {}       ;
    X                   x                   {}       ;
    Y                   y                   {}       ;
    CV_VALUE            cv_value            {}       ;
    DETAIL_RAW_VALUE    detail_raw_value    {}       ;

//    std::vector<MEAN>   detail_means    ;
};
// class StitchedGrid;
template<
      class PROBE_ID_MAPPER
    , class MEAN
    , class STDDEV
    , class PIXELS
    , class POINT
    , class CV_VALUES
    , class DETAIL_RAW_VALUES
>
struct PartialGrid
{
    using CellType = Cell<
          std::size_t   
        , float
        , float         
        , uint16_t
        , uint16_t      
        , uint16_t
        , float // cv
        , cv::Mat_<int32_t>
    >;
    auto operator()( const POINT& p ) const
    {
        auto x = p.x - this->st_point_.x;
        auto y = p.y - this->st_point_.y;
        assert( y < this->mean_.rows );
        assert( x < this->mean_.cols );
        auto&& drv = detail_raw_values_.size() == 0 
            ? cpt::view::make_row_major_view( 
                  detail_raw_values_
                , 0 
                , 0  
            ) 
            : cpt::view::make_row_major_view( 
                  detail_raw_values_
                , mean_.cols
                , mean_.rows 
            );
        // TODO drv : fix empty mean 
        return CellType{
              pidm_->get_id( p.x, p.y )
            , mean_.empty()     ? (float)std::nan("1") 
                : mean_  .template at<float>    ( y, x )
            , stddev_.empty()   ? (float)std::nan("1")   
                : stddev_.template at<float>    ( y, x )
            , pixels_.empty()   ? (uint16_t)0  
                : pixels_.template at<uint16_t> ( y, x )
            , (uint16_t)p.x
            , (uint16_t)p.y
            , cv_values_.empty() ? (float)std::nan("1")
                : cv_values_.template at<float> ( y, x )
            , detail_raw_values_.size() == 0 ? cv::Mat_<int32_t>() : drv( y, x )
            // , src_(tiles_.at( 
            //     y * this->mean_.cols + x 
            // ))
        };
    }
    bool is_in( const POINT& p ) const 
    {
        auto x = p.x - this->st_point_.x;
        auto y = p.y - this->st_point_.y;

        return x >= 0 
            && y >= 0 
            && x < mean_.cols 
            && y < mean_.rows
        ;
    }
    // auto stitch( StitchedGrid& sg )
    // {}
//  protected:
    PROBE_ID_MAPPER&    pidm_       ;
    MEAN                mean_       ;
    STDDEV              stddev_     ;
    PIXELS              pixels_     ;
    POINT               st_point_   ;
    CV_VALUES           cv_values_  ;
    DETAIL_RAW_VALUES    detail_raw_values_;
};

template<
      class PROBE_ID_MAPPER
    , class MEAN
    , class STDDEV
    , class PIXELS
    , class POINT
    , class CV_VALUES
    , class DETAIL_RAW_VALUES
>
auto make_partial_grid(
      PROBE_ID_MAPPER&  pim
    , MEAN&&            mean
    , STDDEV&&          stddev
    , PIXELS&&          pixels
    , POINT&&           point
    , CV_VALUES&&       cv_values
    , DETAIL_RAW_VALUES&& detail_raw_values
)
{
    return PartialGrid<
          PROBE_ID_MAPPER
        , MEAN             
        , STDDEV           
        , PIXELS           
        , POINT            
        , CV_VALUES
        , DETAIL_RAW_VALUES
    >
    {
          pim
        , FWD(mean)
        , FWD(stddev)
        , FWD(pixels)
        , FWD(point)
        , FWD(cv_values)
        , FWD( detail_raw_values )
    };
}
template<class PROBE_ID_MAPPER, class BUF, class POINT>
auto make_partial_grid(
      PROBE_ID_MAPPER&  pim
    , BUF&&             buf
    , POINT&&           point
)
{
    return make_partial_grid(
          pim
        , FWD( buf.mean   )
        , FWD( buf.stddev )
        , FWD( buf.pixels )
        , FWD( point )
        , FWD( buf.cv_mat )
        , FWD( buf.detail_raw_values )
        // , FWD( buf.margin_tiles)
        // , FWD( buf.src    )
    );

};


}}}
