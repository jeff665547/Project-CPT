#pragma once

#include <CPT/algorithm/allele_avg.hpp>
#include <CPT/algorithm/allele_max.hpp>
#include <CPT/algorithm/allele_min.hpp>
#include <boost_addon/range_eval.hpp>
#include <boost_addon/range_sink.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <armadillo>
#include <boost_addon/pipef.hpp>

namespace cpt {
namespace algorithm {

namespace ba = boost::adaptors;
class LogTransform
{
  public:
    template<
          class SOURCE
        , class SINK
        , class MIN
        , class MAX
        , class AVG
        , class CUBE
    >
    auto get_min_max_avg( 
          SOURCE&& source
        , SINK& sink
        , MIN& min
        , MAX& max
        , AVG& avg
        , CUBE&& cube ) const 
    {
        std::size_t avg_num(0);
        auto& allele_signals = cube;
        min = { 
              std::numeric_limits<double>::max()
            , std::numeric_limits<double>::max()
        };
        max = { 
              std::numeric_limits<double>::min()
            , std::numeric_limits<double>::min()
        };
        auto min_ftr ( make_allele_min ( min ) );
        auto max_ftr ( make_allele_max ( max ) );
        auto avg_ftr ( make_allele_avg ( avg, avg_num ) );
        source
            | ba::transformed(
                [] ( auto&& d )
                { 
                    return arma::vec({
                          std::log(d[0])
                        , std::log(d[1])
                    });
                }
            )
            | ba::transformed( min_ftr )
            | ba::transformed( max_ftr )
            | ba::transformed( avg_ftr )
            | ::make_range_sink(sink)
        ;
        avg = avg_ftr.get_result();
    }
    template<
          class SINK
        , class MIN
        , class MAX
        , class AVG
        , class CUBE
    >
    auto get_min_max_avg( 
          arma::mat source
        , SINK& sink
        , MIN& min
        , MAX& max
        , AVG& avg
        , CUBE&& cube ) const
    {
        std::size_t avg_num(0);
        auto& allele_signals = cube;
        min = { 
              std::numeric_limits<double>::max()
            , std::numeric_limits<double>::max()
        };
        max = { 
              std::numeric_limits<double>::min()
            , std::numeric_limits<double>::min()
        };
        auto min_ftr ( make_allele_min ( min ) );
        auto max_ftr ( make_allele_max ( max ) );
        auto avg_ftr ( make_allele_avg ( avg, avg_num ) );
        sink = source;
        sink.each_col(
            [&min_ftr, &max_ftr, &avg_ftr](arma::vec& v)
            {
                v = min_ftr ( v );
                v = max_ftr ( v );
                v = avg_ftr ( v );
            }
        );
        avg = avg_ftr.get_result();
    }
    template<
          class SOURCE
        , class MIN
        , class MAX
        , class AVG
        , class CUBE
    >
    auto get_min_max_avg( 
          SOURCE&& source
        , MIN& min
        , MAX& max
        , AVG& avg
        , CUBE&& cube ) const
    {
        using Sink = std::decay_t<SOURCE>;
        Sink sink;
        get_min_max_avg(
              std::forward<SOURCE>(source)
            , sink
            , min
            , max
            , avg
            , std::forward<CUBE>(cube)
        );
        return sink;
    }
};

}}
