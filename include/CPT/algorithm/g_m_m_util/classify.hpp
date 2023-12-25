#pragma once
#include <utility>
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <Nucleona/container/vector.hpp>
namespace cpt {
namespace algorithm {
namespace g_m_m_util {
template<class MOD, class OB>
auto classify( MOD&& mod, OB&& ob )
{
    arma::Row<std::size_t> labels(ob.n_cols);
    mod.Classify( std::forward<OB>(ob), labels );
    return labels;
}

}}}
