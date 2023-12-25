#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/engine/data_pool.hpp>

namespace cpt {
namespace component {

class DummyAlleleSummarization
    : public engine::NamedComponent
{
  private:
    using Base = engine::NamedComponent;
    using Base::Base;
    size_t num_samples_;
    size_t num_probesets_;

  protected:
    virtual void config_parameters(const bpt::ptree& p) override
    {
        num_probesets_ = p.get<size_t>("num_probesets");
        num_samples_   = p.get<size_t>("num_samples");
    }

  public:
    virtual void initialize() override
    {
        auto& db = this->mut_data_pool();
        db.make_allele_signals(num_probesets_, num_samples_);
    }
};

} // end of namespace component
} // end of namespace cpt
