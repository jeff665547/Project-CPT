#pragma once
#include <algorithm>
#include <cmath>
#include <armadillo>
#include <CPT/range.hpp>
#include <boost/range/combine.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <CPT/algorithm/g_m_m_util/access.hpp>
#include <CPT/logger.hpp>
namespace cpt {
namespace algorithm {
namespace dist_constrain {
RANGE_NAMESPACE_SHORTCUT
namespace mg = mlpack::gmm;
namespace cagu = ca::g_m_m_util;
template<class INT = int64_t, INT MAG = 10000>
struct Fan /* only 2 dimension work */
{
    const std::vector<INT> anchor;

    template<class GRAND_MDL, class LOG>
    Fan( GRAND_MDL&& grand_mdl, LOG& logger)
    : anchor ( trans_mean( grand_mdl | cagu::meaned(), logger ) )
    {
        cpt::dbg << "grand_mdl means : ";
        for ( auto&& a : anchor )
            cpt::dbg << (double)a / MAG << '\t';
        cpt::dbg << std::endl;
    }

    template<class MEANS, class LOG>
    auto trans_mean(MEANS&& means, LOG& logger)
    {
        return means 
            | ba::transformed( [&logger] ( auto&& m )
            {
                auto n = trans( m.at(0), m.at(1) );
                logger << "trans : " << (double)(n) / MAG<< std::endl;
                return n;
            })
            | ::to_vector
            | ca::sort()
        ;
    }

    template<class A, class B>
    static auto trans( A&& a, B&& b ) /* CCS transform */
    {
        return (INT)( ( ( a - b ) / double( a + b ) ) * MAG );
    }
    template< class MEANS, class LOG>
    auto score_match( MEANS&& means, LOG& logger )
    {
        auto sorted_trans_v 
            = means 
            | ba::transformed( [&logger] ( auto&& m )
            {
                auto n = trans( m.at(0), m.at(1) );
                logger << "trans : " << (double)(n) / MAG<< std::endl;
                return n;
            })
            | ::to_vector
            | ca::sort()
        ;
        INT min_dis = std::numeric_limits<INT>::max();
        auto matched_start = 0;
        for ( auto&& i : cu::irange_0( 
            anchor.size() + 1 - sorted_trans_v.size() 
        ))
        {
            INT sum = 0;
            for ( auto&& j : cu::irange_0( sorted_trans_v.size()) )
            {
                auto&& anchor_id = i + j;
                auto dis = std::abs( sorted_trans_v.at(j) - anchor.at(anchor_id) );
                // if ( anchor_id == 0 || anchor_id == 2 )
                //     dis *= 0.7;
                sum += dis;
                
            }
            if ( sum < min_dis )
            {
                min_dis = sum;
                matched_start = i;
            } 
        }
        return std::make_tuple(
              ((double)(min_dis / sorted_trans_v.size())) / MAG
            , matched_start
        );
    }
    template<class OB, class LOG>
    auto score( 
          const mg::GMM& gmm
        , OB&& ob
        , LOG& logger )
    {
        return std::get<0>(score_match ( 
            gmm | cagu::meaned()
            , logger
        ));

    }
};


}}}
