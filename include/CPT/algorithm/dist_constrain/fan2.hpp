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
struct Fan2 /* only 2 dimension work */
{
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
    template<class OB, class LOG>
    auto score( 
          const mg::GMM& gmm
        , OB&& ob
        , LOG& logger )
    {
        if ( gmm.Gaussians() == 1 )
            return 0.001;
        else 
        {
            auto sorted_trans_v 
                = gmm
                | cagu::meaned() 
                | ba::transformed( [&logger] ( auto&& m )
                {
                    auto n = trans( m.at(0), m.at(1) );
                    logger << "trans : " << (double)(n) / MAG<< std::endl;
                    return n;
                })
                | ::to_vector
                | ca::sort()
            ;
            INT min_dis = 1 * MAG;
            for ( std::size_t i (1); i < sorted_trans_v.size(); i ++ )
            {
                auto dis = std::abs(sorted_trans_v.at(i - 1) - sorted_trans_v.at(i));
                if ( dis < min_dis )
                    min_dis = dis;
            }
            return (min_dis * (sorted_trans_v.size() - 1)) / (double)MAG;
        }
    }
};


}}}
