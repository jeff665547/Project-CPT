#ifndef CPT_UTILITY_TPLDS_ALGO_HPP
#define CPT_UTILITY_TPLDS_ALGO_HPP
#include <CPT/utility/tplds/t_pack.hpp>
#include <type_traits>
#include <CPT/utility/tplds/const_value.hpp>
namespace cpt {
namespace utility {
namespace tplds {

template<class T, class GOOD, T... o> 
struct PackRemoveZeroImpl
{};
template<class T, T... good, T o1, T... o>
struct PackRemoveZeroImpl<T, TPack<T, good...>, o1, o... >
{
    using Result = typename std::conditional<
        o1 == zero<T>
        , typename PackRemoveZeroImpl<T, TPack<T, good...>, o...>::Result
        , typename PackRemoveZeroImpl<T, TPack<T, good..., o1>, o...>::Result
    >::type;
};
template<class T, T... good>
struct PackRemoveZeroImpl<T, TPack<T, good...>>
{
    using Result = TPack<T, good...>;
};
template<class T, T... o>
using PackRemoveZero = typename PackRemoveZeroImpl<T, TPack<T>, o...>::Result;

}}}
#endif
