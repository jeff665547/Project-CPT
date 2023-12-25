#pragma once
#include <vector>
#include <armadillo>
#include <utility>

namespace cpt {
namespace component {
namespace birdseed {

using ProbesetIds       = std::vector<
    std::pair<
          std::size_t   /* probeset */
        , std::size_t   /* sample */
    >
>;
/* 2 element probeset, sample */
using AlleleSignal      = arma::vec; 
using ProbesetSignal    = std::vector<AlleleSignal>;
using LabelIdx          = std::vector<std::string>;

}}}
