#ifndef CPT_UTILITY_TPLDS_HPP
#define CPT_UTILITY_TPLDS_HPP
#include <CPT/utility/tplds/t_pack.hpp>
namespace cpt {
namespace utility {

using tplds::TPack;

template<class... T>
struct TypeList{};
}}

#include <CPT/utility/tplds/algo.hpp>
#include <CPT/utility/tplds/string.hpp>
#endif
