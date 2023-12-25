#pragma once
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <CPT/forward.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class DummyAlleleSignals
{
  public:
    arma::Cube<double> allele_signals;

  public:
    template <class DB>
    DummyAlleleSignals(DB&)
    {}

    void make_allele_signals(
        size_t num_probesets
      , size_t num_samples
    ) {
        mlpack::Log::Info << "make allele signals with\n"
                          << num_probesets << " probesets\n"
                          << num_samples   << " samples\n";
        std::default_random_engine gen(1000);
        std::normal_distribution<double> dist(0, 20);
        allele_signals.set_size(2, num_samples, num_probesets);
        for (size_t i = 0; i != num_probesets; ++i)
        {
            mlpack::gmm::GMM model(3, 2);
            model.Component(0).Mean() = { 2100 + dist(gen),  100 + dist(gen) };
            model.Component(1).Mean() = { 1100 + dist(gen), 1100 + dist(gen) };
            model.Component(2).Mean() = {  100 + dist(gen), 2100 + dist(gen) };
            model.Component(0).Covariance(arma::eye(2, 2) * 50);
            model.Component(1).Covariance(arma::eye(2, 2) * 50);
            model.Component(2).Covariance(arma::eye(2, 2) * 50);
            model.Weights() = arma::randu<arma::Col<double>>(3);
            model.Weights() /= arma::accu(model.Weights());
            auto& slice = allele_signals.slice(i);
            for (size_t j = 0; j != num_samples; ++j)
                slice.col(j) = model.Random();
        }
        mlpack::Log::Info << "done";
    }
};

}
}
}
