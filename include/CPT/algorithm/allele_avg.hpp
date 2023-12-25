#pragma once
#include <CPT/algorithm/state_scan.hpp>


namespace cpt {
namespace algorithm {
struct AlleleAvgFtr
{
    std::size_t& nums;
    AlleleAvgFtr(std::size_t& n)
    : nums(n)
    {}

    template<class STATE, class __IN>
    // std::enable_if_t<
    //     ::decay_equiv_v<STATE, IN>
    // > 
    auto operator () ( STATE&& state, __IN&& in) const 
    {
        state += in;
        nums ++                     ;
    }
};
template<class STATE>
using AlleleAvgBase = StateScan<STATE, AlleleAvgFtr>;
template < class STATE >
struct AlleleAvg 
: public AlleleAvgBase<STATE>
{
    using Base = AlleleAvgBase<STATE>;
    AlleleAvg( STATE&& ini_state, std::size_t& n)
    : Base ( 
          std::forward<STATE>( ini_state )
        , AlleleAvgFtr(n)
    )
    {}
    auto get_result() const
    {
        return std::remove_reference_t<STATE>({
              Base::state[0] / Base::do_.nums
            , Base::state[1] / Base::do_.nums
        });
    }
};
template<class STATE>
auto make_allele_avg( STATE&& ini_state, std::size_t& n) 
{
    return AlleleAvg<STATE>(
          std::forward<STATE>( ini_state ), n
    );
}
}}
