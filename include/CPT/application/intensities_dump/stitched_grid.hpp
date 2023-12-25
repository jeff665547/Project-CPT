#pragma once
#include <opencv2/opencv.hpp>
#include <CPT/application/intensities_dump/probe_id_mapper.hpp>
#include <iostream>
#include <CPT/application/intensities_dump/point.hpp>
#include <CPT/application/intensities_dump/utility.hpp>
#include <CPT/logger.hpp>
#include <CPT/application/intensities_dump/partial_grid.hpp>
#include <CPT/format/chip_sample.hpp>
#include <Nucleona/language.hpp>
// #include <CPT/spec/denali/const.hpp>

namespace cpt { namespace application {
namespace intensities_dump {
namespace cf_ = cpt::format;
namespace cfcs_ = cf_::chip_sample;
template<class T> struct StitchedGridTo;

template<
      class PROBE_ID_MAPPER
    , class MEAN
    , class STDDEV
    , class PIXELS
    , class POINT
    , class CV_VALUES
    , class DETAIL_RAW_VALUES
//    , class TILES
//    , class SRC
>
struct StitchedGrid : public std::vector< PartialGrid<  
      PROBE_ID_MAPPER
    , MEAN
    , STDDEV
    , PIXELS
    , POINT
    , CV_VALUES
    , DETAIL_RAW_VALUES
//     , TILES
//     , SRC
>> 
{
    using Base          = std::vector< PartialGrid<  
          PROBE_ID_MAPPER
        , MEAN
        , STDDEV
        , PIXELS
        , POINT
        , CV_VALUES
        , DETAIL_RAW_VALUES
//         , TILES
//         , SRC
    >>; 
    using SubGrid       = typename Base::value_type;
    using result_type   = typename SubGrid::CellType;

    struct OverlapCellCollect : public result_type 
    {
        std::vector<typename std::decay_t<MEAN>::value_type>   detail_means    ;
        int num         {0}                 ; // TODO remove
        auto push_back ( const result_type& o ) 
        {
            detail_means.push_back( o.mean );
            if ( num == 0 )
            {
                static_cast<result_type&>(*this) = o;
            }
            else
            {
                // TODO stddev pixels
                // this->mean += o.mean;
                if ( this->cv_value > o.cv_value )
                {
                    static_cast<result_type&>(*this) = o;
                }
                assert ( this->probe_id == o.probe_id );
                assert ( this->x == o.x );
                assert ( this->y == o.y );
            }
            num ++;
        }
        auto& get_result() 
        {
            assert ( num > 0 );
            // this->mean /= num;
            for( auto v : result_type::detail_raw_value )
            {
                assert( v >= 0 );
            }
            return *this;
        }
    };

    StitchedGrid() = default;
    StitchedGrid( uint16_t r, uint16_t c )
    : n_rows_( r )
    , n_cols_( c )
    {}
    auto operator()( const POINT& p ) const 
    {
        OverlapCellCollect occ;
        for(auto&& grid : *this)
        {
            if ( grid.is_in ( p ) )
            {
                auto&& r ( grid.operator()( p ) );
                occ.push_back( r );
            }
        }
        return occ.get_result();
    }
    auto operator() ( 
          const decltype(std::decay_t<POINT>::x)& x
        , const decltype(std::decay_t<POINT>::y)& y
    )
    {
        return operator()( std::decay_t<POINT>{ x, y } );
    }
    auto n_cols()
    {
        return n_cols_;
    }
    auto n_rows()
    {
        return n_rows_;
    }
    template<class T, class... ARGS>
    auto to( ARGS&&... args )
    {
        return StitchedGridTo<std::decay_t<T>>::convert( *this, FWD(args)... );
    }
  protected:
    uint16_t n_rows_        {0};
    uint16_t n_cols_        {0};
};
template<class T>
struct StitchedGridTo
{
    template<class SG>
    static auto convert(SG&& sg)
    {
        throw std::runtime_error("unsupported conversion");
    }
};
template<>
struct StitchedGridTo<cpt::format::chip_sample::Array>
{
    template<class SG>
    static auto convert(SG&& sg)
    {
        cfcs_::ArrayReader<cf_::ProbeGrid> array_reader;
        return array_reader( FWD(sg) );
    }
};
template< class PG >
struct GetStitchedGridTypeImpl
{};
template<
      class PROBE_ID_MAPPER
    , class MEAN
    , class STDDEV
    , class PIXELS
    , class POINT
    , class CV_VALUES
    , class DETAIL_RAW_VALUES
//    , class TILES
//    , class SRC
>
struct GetStitchedGridTypeImpl<PartialGrid<
      PROBE_ID_MAPPER
    , MEAN
    , STDDEV
    , PIXELS
    , POINT
    , CV_VALUES
    , DETAIL_RAW_VALUES
//     , TILES
//     , SRC
>>
{
    using Result = StitchedGrid<
          PROBE_ID_MAPPER
        , MEAN
        , STDDEV
        , PIXELS
        , POINT
        , CV_VALUES
        , DETAIL_RAW_VALUES
//         , TILES
//         , SRC
    >;
};
template<class PG>
using GetStitchedGridType = typename GetStitchedGridTypeImpl<PG>::Result;
    

}}}
