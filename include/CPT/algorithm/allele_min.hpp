#pragma once
#include <CPT/algorithm/state_scan.hpp>


namespace cpt {
namespace algorithm {
struct AlleleMinFtr
{
    template<class STATE, class __IN>
    // std::enable_if_t<
    //     ::decay_equiv_v<STATE, IN>
    // > 
    auto operator () ( STATE&& state, __IN&& in) const 
    {
        if( state[0] > in[0] )
        {
            state[0] = in[0];
        }
        if ( state[1] > in[1] )
        {
            state[1] = in[1];
        }
    }
};

template<class STATE>
using AlleleMin = StateScan<STATE, AlleleMinFtr>;

template<class STATE>
auto make_allele_min( STATE&& ini_state )
{
    return AlleleMin<STATE>(
          std::forward<STATE>( ini_state )
        , AlleleMinFtr()
    );
}
}}
