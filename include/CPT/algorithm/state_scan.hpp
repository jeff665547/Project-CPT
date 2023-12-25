#pragma once
#include <utility>
#include <type_traits>
#include <boost/type_traits.hpp>
#include <Nucleona/type_traits/core.hpp>
template<class STATE>
struct StateScanBase
{
    mutable STATE state;
};

template<class STATE>
struct StateScanBase<STATE&>
{
    STATE& state;
};
template<class FUN>
using ft = boost::function_traits<FUN>;
template<class STATE, class DO>
class StateScan : public StateScanBase<STATE>
{
  protected:
    DO do_;
  public:
    using Base = StateScanBase<STATE>;
    StateScan( STATE&& ini_state, DO&& _do )
    : Base { std::forward < STATE > ( ini_state ) }
    , do_  { std::forward < DO    > ( _do ) }
    {}

    template<class T>
    T operator()( T&& in ) const 
    {
        do_(Base::state, in);
        return std::forward<T>(in);
    }
    auto get_result() const
    {
        return Base::state;
    }
};

