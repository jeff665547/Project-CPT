#pragma once 
#ifdef NEW_DATA_POOL
#include <CPT/components/genotyping2.hpp>
#else
#include <CPT/logger.hpp>
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/birdseed.hpp>
#include <iostream>
#include <iomanip>
#include <CPT/logger.hpp>
#ifdef PROBESET_TRAINING_PLOT 
#include <CPT/visualization/magic.hpp>
#endif
#include <CPT/algorithm/matrix_range.hpp>
#include <CPT/range.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <CPT/format/json.hpp>
#include <set>

namespace cpt {
namespace component {
#ifdef PROBESET_TRAINING_PLOT 
namespace cptv = cpt::visualization;
#endif
namespace ca = cpt::algorithm;
namespace ba = boost::adaptors;
namespace ced = cpt::engine::data_pool;
namespace cf = cpt::format;
template <class POLICY, bool training = false>
class Genotyping : public cpt::engine::NamedComponent
{
    using Base = cpt::engine::NamedComponent;
    using Base::Base;
};

template <>
class Genotyping<ca::Birdseed, false>
    : public cpt::engine::NamedComponent
{
  public:
    using Base = engine::NamedComponent;
    using Base::Base;
    using Policy = cpt::algorithm::Birdseed;

  private:
    std::shared_ptr<format::Tsv<>> results_;
    std::shared_ptr<format::Tsv<>> summary_of_probesets_;
    std::shared_ptr<std::mutex> mutex_;
    std::set<std::string> dbg_view_list_;
  private:
    virtual void config_parameters(const bpt::ptree& p) override
    {
        auto& db = this->mut_data_pool();

        using TypeAlias1 = decltype(results_)::element_type;
        results_ = db.require_shared<TypeAlias1>("Genotypes");
        results_->register_column("probeset_name");
        results_->register_column("sample_name");
        results_->register_column("a_allele");
        results_->register_column("b_allele");
        results_->register_column("genotype");
        results_->register_column("posterior");

        using TypeAlias2 = decltype(summary_of_probesets_)::element_type;
        summary_of_probesets_ = db.require_shared<TypeAlias2>("SummaryOfProbesets");
        summary_of_probesets_->register_column("sample_name");
        summary_of_probesets_->register_column("call_rate");

        auto jp = cf::make_json(p);
        auto dvl ( jp.get_optional<std::string>("dbg_view_list").value_or("") );
        if( dvl != "" )
        {
            std::ifstream fin(dvl);
            std::string line;
            while ( std::getline(fin, line))
                dbg_view_list_.emplace ( std::move( line ) );
        }
    }

  public:
    virtual void initialize(void) override
    {
        auto& db = this->mut_data_pool();
        db.require_clustering_models(db);

        auto num_samples = db.cube.n_cols;
        auto num_probesets = db.cube.n_slices;
        db.genotypes.set_size(num_probesets, num_samples);
        db.likelihoods.copy_size(db.genotypes);
        db.posteriors.copy_size(db.genotypes);

        mutex_.reset(new std::mutex);
    }

    virtual void start(void) override
    {
        auto& logger = cpt::dbg;
        auto& db = this->mut_data_pool();
        auto& allele_signals = db.cube;
        const auto num_probesets = allele_signals.n_slices;
        const auto num_samples = allele_signals.n_cols;
        const auto job_pack = 2000;

        for (auto first = 0u; first < num_probesets; first += job_pack)
        {
            auto last = first + job_pack;
            if (last > num_probesets)
                last = num_probesets;

            db.thread_pool->job_post([&, first, last](void)
            {
                std::unique_lock<std::mutex> lock(*mutex_, std::defer_lock);

                arma::Row<size_t> genotypes(num_samples);
                arma::Row<double> likelihoods(num_samples);
                arma::Row<double> posteriors(num_samples);

                for (auto i = first; i != last; ++i)
                {
                    if (!db.validities[i])
                        continue;
                    
                    // lock.lock();
                    // cpt::info << "[" << i << "]" << std::flush;
                    // lock.unlock();
                    for ( auto& e : db.cube.slice(i) ) e = log(e);
                    auto&& mdl = db.models[i];
                    auto&& obs = db.cube.slice(i);
                    #ifdef PROBESET_TRAINING_PLOT 
                    auto ps_name = db.cube.z_axis.search_label_at(i);
                    bool plot_find( dbg_view_list_.find( ps_name ) != dbg_view_list_.end() );
                    {
                        if( plot_find )
                        {
                            std::lock_guard<std::mutex>lock(*mutex_);
                            logger << ps_name << std::endl;           
                            cptv::ScatterPlot view; 
                            view.show_legend = false;
                            view.initialize(); 
                            cptv::show_plot_sample( view, obs );
                            cptv::show_plot_model ( view, mdl );
                            view.render_and_start();
                        }
                    }
                    #endif
                    Policy::infer(
                        obs
                      , mdl
                      , genotypes
                      , likelihoods
                      , posteriors
                    );
                    #ifdef PROBESET_TRAINING_PLOT 
                    {
                        if ( plot_find )
                        {
                            std::lock_guard<std::mutex>lock(*mutex_);
                            cptv::ScatterPlot view; 
                            view.show_legend = false;
                            view.initialize(); 
                            cptv::show_plot_sample( view, obs, genotypes);
                            cptv::show_plot_model ( view, mdl );
                            view.render_and_start();
                        }
                    }
                    #endif
                    lock.lock();
                    db.genotypes.row(i) = genotypes;
                    db.likelihoods.row(i) = likelihoods;
                    db.posteriors.row(i) = posteriors;
                    lock.unlock();
                }
            });
        }
        db.thread_pool->flush_pool();
                
        double total = 0.0;
        std::vector<double> num_calls(num_samples, 0.0);
        const auto& probeset_names = allele_signals.z_axis.get_labels();
        const auto& sample_names = allele_signals.y_axis.get_labels();
        for (size_t i = 0; i != num_probesets; ++i)
        {
            bool flag = db.validities[i];
            // if (flag)
                total += 1.0;

            for (size_t j = 0; j != num_samples; ++j)
            {
                results_->push_entry(
                    probeset_names[i]
                  , sample_names[j]
                  , allele_signals(0, j, i)
                  , allele_signals(1, j, i)
                  , static_cast<int32_t>(flag? db.genotypes(i, j): -2)
                  , static_cast<double>(flag? db.posteriors(i, j): 0)
                );
                if (flag and db.posteriors(i, j) > 0.7)
                    num_calls[j] += 1.0;
            }
        }

        const auto& sample_vec = allele_signals.y_axis.get_labels();

        for (size_t j = 0; j != num_samples; ++j)
        {
            summary_of_probesets_->push_entry(
                sample_vec[j]
              , num_calls[j] / total
            );
        }
    }

    virtual void finish(void) override
    {
        auto& db = this->mut_data_pool();
        mutex_.reset();

        // export genotype results
        {
            auto path = db.output_dir();
            path += "genotype.tsv";
            auto& node = db.require_output("genotype");
            db.add_output_path(node, path);

            std::ofstream os(path.string());
            results_->ref()
                .select(
                    "probeset_name"
                  , "sample_name"
                  , "a_allele"
                  , "b_allele"
                  , "genotype"
                  , "posterior"
                )
                .where([](const auto&){ return true; })
                .view()
                .dump(os);

            os.close();
            results_.reset();
        }

        // export summary of probesets
        {
            auto path = db.output_dir();
            path += "call_rate.tsv";
            auto& node = db.require_output("call_rate");
            db.add_output_path(node, path);

            std::ofstream os(path.string());
            summary_of_probesets_->ref()
                .select(
                    "sample_name"
                  , "call_rate"
                )
                .where([](const auto&){ return true; })
                .view()
                .dump(os);

            os.close();
            summary_of_probesets_.reset();
        }
    }
};

template <>
class Genotyping<cpt::algorithm::Birdseed, true>
    : public cpt::engine::NamedComponent
{
  public:
    using Base = engine::NamedComponent;
    using Base::Base;
    using Policy = cpt::algorithm::Birdseed;

  private:
    size_t maxiter_;
    double convtol_;
    size_t trials_;
    bool use_existing_model_;
    std::shared_ptr<std::mutex> mutex_;

  protected:
    virtual void config_parameters(const bpt::ptree& p) override
    {
        maxiter_ = p.get_optional<size_t>("maxiter").get_value_or(200);
        convtol_ = p.get_optional<double>("convtol").get_value_or(1e-4);
        trials_  = p.get_optional<size_t>("trials") .get_value_or(1);
        use_existing_model_ = p.get_optional<bool>("use_existing_models")
                               .get_value_or(false);
    }

  public:
    virtual void initialize() override
    {
        auto& db = this->mut_data_pool();
        auto& allele_signals = db.cube;
        if (use_existing_model_)
        {
            db.require_clustering_models(db);
            return;
        }
        for (size_t i = 0; i != allele_signals.n_slices; ++i)
        {
            db.models.emplace_back(mlpack::gmm::GMM(3, 2));
        }

        auto num_samples = db.cube.n_cols;
        auto num_probesets = db.cube.n_slices;
        db.genotypes.set_size(num_samples, num_probesets);
        db.likelihoods.copy_size(db.genotypes);
        db.posteriors.copy_size(db.genotypes);
        mutex_.reset(new std::mutex);
    }

    virtual void start() override
    {
        auto& db = this->mut_data_pool();
        auto& allele_signals = db.cube;
        const auto num_probesets = allele_signals.n_slices;
        const auto num_samples = allele_signals.n_cols;

        db.validities.resize(num_probesets, false);
        const static int job_pack = 2000;

        for (auto first = 0u; first < num_probesets; first += job_pack)
        {
            auto last  = first + job_pack;
            if (last > num_probesets)
                last = num_probesets;

            db.thread_pool->job_post([&, first, last](void) noexcept
            {
                std::unique_lock<std::mutex> lock(*mutex_, std::defer_lock);

                lock.lock();
                std::cerr << num_probesets << '\n'
                          << db.models.size() << '\n';
                lock.unlock();

                for(auto i = first; i != last; ++i)
                {
                    auto& model = db.models[i];
                    lock.lock();
                    cpt::msg << "[" << i << "]" << std::flush;
                    lock.unlock();

                    // Train model
                    Policy::train(
                        db.cube.slice(i)
                      , model
                      , maxiter_
                      , convtol_
                      , trials_
                      , use_existing_model_
                    );

                    // Model validation
                    double min_w = arma::min(model.Weights());
                    bool valid = min_w > (3.0 / num_samples);
                    if (!valid)
                    {
                        lock.lock();
                        cpt::warn << "\n[WARN: " << i << "] min(w) = " << min_w << std::endl;
                        lock.unlock();
                        continue;
                    }

                    // sort clusters based on the value of atan(A/B)
                    // descending order
                    std::vector<size_t> inds(3);
                    std::iota(inds.begin(), inds.end(), 0);
                    std::sort(
                        inds.begin()
                      , inds.end()
                      , [&model](const auto& i, const auto& j)
                        {
                            auto&& u1 = model.Component(i).Mean();
                            auto&& u2 = model.Component(j).Mean();
                            return std::atan(u1[0] / u1[1]) >
                                   std::atan(u2[0] / u2[1]) ;
                        }
                    );
                    arma::vec weights(3);
                    std::vector<mlpack::distribution::GaussianDistribution> gaussians(3);
                    for (size_t k = 0; k != 3; ++k)
                    {
                        gaussians[k] = model.Component(inds[k]);
                        // weights(k)   = 1.0 / 3.0;
                        weights(k)   = model.Weights()(inds[k]);
                    }

                    lock.lock();
                    model = mlpack::gmm::GMM(gaussians, weights);
                    db.validities[i] = valid;
                    lock.unlock();
                }
            });
        }
        db.thread_pool->flush_pool();
    }

    virtual void finish() override
    {
        auto& db = this->mut_data_pool();
        db.save_clustering_models(db);
        mutex_.reset();
    }
};

}
}
#endif 
