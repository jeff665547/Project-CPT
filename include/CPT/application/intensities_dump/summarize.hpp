#pragma once
#include <opencv2/opencv.hpp>
#include <CPT/application/intensities_dump/probe_id_mapper.hpp>
#include <iostream>
#include <CPT/application/intensities_dump/point.hpp>
#include <CPT/application/intensities_dump/utility.hpp>
#include <CPT/logger.hpp>
// #include <CPT/application/intensities_dump/spec.hpp>
namespace cpt { namespace application {
namespace intensities_dump {

template<class PROBE_ID_MAPPER, class GRID, class POINT>
struct SummarizeGrid
{
    using GridType = GRID;
    using PointType = POINT;
    using ProbeIdMapper = PROBE_ID_MAPPER;
    struct Result
    {
        std::size_t probe_id;
        float       intensity {0};
        int         x;
        int         y;
        std::vector<float> detail_intensities;
        cv::Mat_<int32_t> pixels;
        float       cv_value;
    };
    SummarizeGrid( ProbeIdMapper& pidm, GRID&& grid, POINT&& point )
    : pidm_ ( pidm )
    , grid_ ( std::forward<GRID>(grid) )
    , point_ ( std::forward<POINT> ( point ) ) 
    {
    }
    
    auto operator()( const cv::Point& p ) const
    {
        auto x = p.x - this->point_.x;
        auto y = p.y - this->point_.y;
        // y = this->grid_.rows - 1 - y;
        assert( y < grid_.rows );
        assert( x < grid_.cols );
        return Result {
            pidm_ ( p.x, p.y )
            , grid_.template at<float>( y, x )
            , p.x
            , p.y
        };
    }
    bool is_in( const cv::Point& p ) const 
    {
        auto x = p.x - this->point_.x;
        auto y = p.y - this->point_.y;
        // y = this->grid_.rows - 1 - y;
        return x >= 0 
            && y >= 0 
            && x < grid_.cols 
            && y < grid_.rows;
    }

  private:
    ProbeIdMapper& pidm_;
  public:
    GRID grid_;
  private:
    POINT point_;
};
template<class PIDM, class GRID, class POINT>
struct SummarizeGridGroup : public std::vector<SummarizeGrid<
      PIDM
    , GRID
    , POINT
>>
{
    using Grid = SummarizeGrid<PIDM, GRID, POINT>;

    using result_type = typename Grid::Result;
    auto operator()( const cv::Point& p ) const
    {
        result_type res;
        int num = 0;
        std::vector<float> intensities_of_p;
        for(auto&& grid : *this)
        {
            if ( grid.is_in ( p ) )
            {
                auto&& r ( grid.operator()( p ) );
                // std::cout << __FILE__ << ":" << __LINE__ 
                //     << " " << r.intensity << std::endl;
                intensities_of_p.push_back( r.intensity );
                if ( num == 0 ) 
                {
                    res = std::move(r);
                    num ++;
                }
                else 
                {
                    if ( !std::isnan( r.intensity ) )
                    {
                        res.intensity += r.intensity;

                        assert ( res.probe_id == r.probe_id );
                        assert ( res.x == r.x );
                        assert ( res.y == r.y );
                        num ++;
                    }
                }
            }
        }
        assert ( num > 0 );
        res.intensity /= num;
        res.detail_intensities = std::move( intensities_of_p );
        return res;
    }
    template<class W, class H, class FUNC>
    auto operator()(const W& w, const H& h, FUNC&& f ) const
    {
        // for ( auto&& grid : *this )
        // {
        //     std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        //     show ( 0, [&]
        //     {
        //         auto tmp = grid.grid_.clone();
        //         cv::normalize(tmp, tmp, 0, 1, cv::NORM_MINMAX, CV_32F);
        //         return tmp;
        //     });
        // }
        for ( H i(0); i < h; i ++ )
        {
            for ( W j(0); j < w; j ++ )
            {
                f( this->operator()( cv::Point( j, i ) ) );
                // f ( this->operator()( Point<OutputCoordinateSystem, int>{j, i} ) ); // the domain can be ignored
            }
        }
#ifdef GRID_CHECK
        // using GridCheckParser = cpt::format::TupleParser< cpt::format::TraitPrototype<
        //       std::tuple
        //     , cpt::format::PMT<0, uint32_t>
        //     , cpt::format::PMT<1, uint32_t>
        //     , cpt::format::PMT<2, uint32_t>
        //     , cpt::format::PMT<3, uint32_t>
        // >>;
        // struct GridCheckEntry{ uint32_t x, y, w, h; };
        // std::string grid_check_region = "./grid_check_region.tsv";
        // if ( boost::filesystem::exists( grid_check_region ) )
        // {
        //     std::ifstream fin  ( grid_check_region );
        //     std::ofstream fout ( "./grid_check_result.tsv" );
        //     std::string line;
        //     GridCheckParser parser;
        //     while ( std::getline ( fin, line ) )
        //     {
        //         auto tup = parser( line );
        //         GridCheckEntry gce{ std::get<0>(tup), std::get<1>(tup), std::get<2>(tup), std::get<3>(tup) };
        //         for ( uint32_t i = gce.y; i < gce.y + gce.h; i ++ )
        //         {
        //             for ( uint32_t j = gce.x; j < gce.x + gce.w; j ++ )
        //             {
        //                 fout 
        //                     << j 
        //                     << '\t' << i
        //                     << '\t' << this->operator() ( 
        //                         Point<GridCheckCoordinateSystem, int>{ (int)j, (int)i } 
        //                     )
        //                     << std::endl;
        //             }
        //         }
        //     }
        // }
        // else
        // {
        //     cpt::warn << "the compile flag \"GRID_CHECK\" raised," << std::endl;
        //     cpt::warn << "but no check file \"grid_check_region.tsv\" under the working directory" << std::endl;
        //     cpt::warn << "the check result will not generate." << std::endl;
        // }
#endif
    }
};
struct Summarize
{
    template<class PID_MAPPER, class GRID, class P>
    auto run( PID_MAPPER& pidm, GRID&& grid, P&& p )
    {
        return SummarizeGrid<PID_MAPPER, GRID, P>( 
              pidm
            , std::forward<GRID>( grid ) 
            , std::forward<P>( p )
        );
    }
    
};

}
}}
