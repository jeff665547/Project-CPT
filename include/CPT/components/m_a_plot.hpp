#pragma once 
#include <CPT/engine/components/named_component.hpp>
#include <iostream>
#include <CPT/logger.hpp>
namespace cpt
{
namespace component
{
class MAPlot : public cpt::engine::NamedComponent
{
  public:
    using Base = engine::NamedComponent;
    double k;
    uint32_t size;
  public:
    using Base::Base;
  protected:
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        k    = p.get_optional<double>  ("k")   .get_value_or(0);
        size = p.get_optional<uint32_t>("size").get_value_or(10000);
    }
    void set_outputs(bpt::ptree& ptree)
    {
        auto& db        ( this->mut_data_pool() );
        // auto& output    ( db.result_schema() );
        bpt::ptree child;
        bpt::ptree output;
        for (auto& path: db.get_path_list("tsv_path"))
        {
            child.put("", path.string());
            output.push_back(std::make_pair("", child));
        }
        ptree.add_child("output", output);
    }
  public:
    virtual void initialize() override
    {
        auto& db ( this->mut_data_pool() );
        for (const auto& path : db.get_path_list("cel_path"))
        {
            db.push_path(
                  "tsv_path"
                , db.output_dir() / path.filename().replace_extension("tsv")
            );
        }
        set_outputs( db.result_schema() );
    }
    virtual void start() override
    {
        cpt::msg << "In MAPlot\n";

        std::ofstream f;
        for (
            uint32_t j = 0; 
            j != num_samples(); 
            ++j
        )
        {
            cpt::verbose0 << "plot " << this->data_pool_.get_path_list("tsv_path")[j] << '\n';
            auto& db ( this->mut_data_pool() );
            auto& SI_A = db.signal_intansities[0][j];
            auto& SI_B = db.signal_intansities[1][j];

            std::vector<float> M(SI_A.size(), 0.0), A(SI_B.size(), 0.0);
            if (k == 1)
            {
                cpt::msg << "k = " << k << '\n';
                for (uint32_t i = 0; i != M.size(); ++i)
                {
                    M[i] = (std::log2(SI_A[i]) - std::log2(SI_B[i]));
                    A[i] = (std::log2(SI_A[i]) + std::log2(SI_B[i])) / 2;
                }
            }
            else if (k == 2)
            {
                for (uint32_t i = 0; i != M.size(); ++i)
                {
                    M[i] = (SI_A[i] - SI_B[i]) / (SI_A[i] + SI_B[i]);
                    M[i] = std::sinh(k * M[i] / std::sinh(k));
                    A[i] = std::log2(SI_A[i] + SI_B[i]);
                }
            }
            else
            {
                std::copy(SI_A.begin(), SI_A.end(), M.begin());
                std::copy(SI_B.begin(), SI_B.end(), A.begin());
            }

            f.open(db.get_path_list("tsv_path")[j].string());
            std::vector<uint32_t> indices(this->num_probes());
            std::iota(indices.begin(), indices.end(), 0);
            std::shuffle(indices.begin(), indices.end(), std::random_device());
            f << "x\ty\n";
            for (uint32_t i = 0; i != size; ++i)
            {
                f << M[i] << '\t' << A[i] << '\n';
            }
            f.clear();
            f.close();
        }
    }

};
}
}
