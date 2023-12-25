#pragma once
#include <utility>
#include <mlpack/methods/gmm/em_fit.hpp>

namespace cpt {
namespace algorithm {

namespace mg = mlpack::gmm;

template<class ICT, class COV_T>
auto make_em( 
      const std::size_t&    max_iter
    , const double&         tol
    , ICT&&                 ict
    , COV_T&&               cov
)
{
    return mg::EMFit<ICT, COV_T>(
          max_iter
        , tol
        , std::forward<ICT>     (ict)
        , std::forward<COV_T>   (cov)
    );
}

}}
