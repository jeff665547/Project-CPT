#pragma once
#include <utility>
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <Nucleona/container/vector.hpp>
#include <CPT/algorithm/g_m_m_util/fitting.hpp>
#include <CPT/format/json.hpp>
namespace cpt {
namespace algorithm {
namespace cf = cpt::format;
class GMMFitting
{
  public:
    template< class... ARGS >
    decltype(auto) call_by_dists( ARGS&&... args ) const 
    {
        return g_m_m_util::fitting_by_dists(
            std::forward<ARGS>(args)...
        );
    }
    template< class... ARGS >
    decltype(auto) call_by_means( ARGS&&... args ) const 
    {
        return g_m_m_util::fitting_by_means(
            std::forward<ARGS>(args)...
        );
    }
};

}
}
