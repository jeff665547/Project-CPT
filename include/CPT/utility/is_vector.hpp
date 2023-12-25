#pragma once
#include <type_traits>
#include <CPT/utility/typecheck.hpp>
namespace cpt{ namespace utility{
template<class... R> using StdVector = std::vector<R...>;
CREATE_TYPECHECKER( StdVector );


}}
