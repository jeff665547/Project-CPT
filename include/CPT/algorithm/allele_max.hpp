#pragma once
#include <CPT/algorithm/state_scan.hpp>

namespace cpt {
namespace algorithm {
struct AlleleMaxFtr
{
    template<class STATE, class __IN>
    // std::enable_if_t<
    //     ::decay_equiv_v<STATE, IN>
    // > 
    auto operator () ( STATE&& state, __IN&& in) const 
    {
        if( state[0] < in[0] )
        {
            state[0] = in[0];
        }
        if ( state[1] < in[1] )
        {
            state[1] = in[1];
        }
    }
};

template<class STATE>
using AlleleMax = StateScan<STATE, AlleleMaxFtr>;

template<class STATE>
auto make_allele_max( STATE&& ini_state )
{
    return AlleleMax<STATE>(
          std::forward<STATE>( ini_state )
        , AlleleMaxFtr()
    );
}
}}
