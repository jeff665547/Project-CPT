#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <iostream>
#include <numeric>
#include <random>
namespace cpt
{
namespace component
{
class QuantileNorm : public engine::NamedComponent
{
    using Base = engine::NamedComponent;
public:
    using Base::Base;
    virtual void initialize() override
    {
        auto& db ( this->mut_data_pool() );
        db.require_cdf(db.get_path("cdf_path").string());
        for(auto& path : db.get_path_list("cel_path"))
        {
            db.require_cel(path.string(), db);
        }
    }
    virtual void start() override
    {
        auto& db ( this->mut_data_pool());
        std::vector<std::vector<uint32_t>>    inds[2];
        std::vector<float>                    distr(num_probes(), 0.0);
        std::vector<uint32_t>                 ind(num_probes());
        for (uint32_t channel = 0; channel != 2; ++channel)
        {
            for (uint32_t j = 0; j != num_samples(); ++j)
            {
                auto& SI = db.signal_intansities[channel][j];
                std::iota(ind.begin(), ind.end(), 0);
                std::sort(ind.begin(), ind.end(),
                  [&SI](const uint32_t lhs, const uint32_t rhs)
                    { return SI.at(lhs) < SI.at(rhs); });
                inds[channel].emplace_back(ind);
                
                auto itr = distr.begin();
                for (auto& i: ind)
                    *itr++ += SI[i] / num_samples() / 2;
            } 
        }
        for (uint32_t channel = 0; channel != 2; ++channel)
        {
            for (uint32_t j = 0; j != num_samples(); ++j)
            {
                auto& SI = db.signal_intansities[channel][j];
                auto itr = distr.begin();
                for (auto& i: inds[channel][j])
                    SI[i] = *itr++;
            }
        }
    }
};
}
}
