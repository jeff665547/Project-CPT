#pragma once 
#ifdef NEW_DATA_POOL
#include <CPT/logger.hpp>
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/birdseed.hpp>
#include <CPT/algorithm/axiom_GT.hpp>
#include <CPT/engine/data_pool/axiomGT_auxiliary.hpp>
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
#include <CPT/engine/data_pool/component_object_manager.hpp>

namespace cpt {
namespace component {
#ifdef PROBESET_TRAINING_PLOT 
namespace cptv = cpt::visualization;
#endif
namespace ca = cpt::algorithm;
namespace ba = boost::adaptors;
namespace ced = cpt::engine::data_pool;
namespace cf = cpt::format;
namespace com_ = cpt::engine::data_pool::component_object_manager;
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
    using Tsvs = std::vector< cpt::format::Tsv<> >;

  private:
    com_::ICPObjPtr< cf::Tsv<> > results_               ;
    com_::ICPObjPtr< cf::Tsv<> > summary_of_probesets_  ;
    com_::ICPObjPtr<
        cf::Cube<double>
    >                            probeset_cube_         ;
    std::unique_ptr< std::mutex> mutex_;
    std::set<std::string> dbg_view_list_;
  private:
    virtual void config_parameters(const bpt::ptree& p) override
    {
        using TypeAlias1 = decltype(results_)::element_type;

        auto& db = this->mut_data_pool();
        auto json = cf::make_json( p );
        results_ = com_::require_w< cf::Tsv<> >(
            json, "results", com_::make_ref_parameter("Genotypes")
        );
        results_-> initialize(); 

        results_->get().register_column("probeset_name");
        results_->get().register_column("sample_name");
        results_->get().register_column("a_allele");
        results_->get().register_column("b_allele");
        results_->get().register_column("genotype");
        results_->get().register_column("posterior");

        probeset_cube_ = com_::require_w <cf::Cube<double>> ( 
              json
            , "probeset_cube"         
            ,  com_::make_ref_parameter("probeset_cube")  
        );

        using TypeAlias2 = decltype(summary_of_probesets_)::element_type;
        summary_of_probesets_ = com_::require_w< cf::Tsv<> >(
              json, "summary_of_probesets"
            , com_::make_ref_parameter("SummaryOfProbesets")
        );
        summary_of_probesets_ -> initialize(); 
        summary_of_probesets_->get().register_column("sample_name");
        summary_of_probesets_->get().register_column("call_rate");

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
        probeset_cube_        -> initialize();
        db.require_clustering_models(db);

        auto& probeset_cube = probeset_cube_ -> get();
        auto num_samples    = probeset_cube.n_cols;
        auto num_probesets  = probeset_cube.n_slices;
        db.genotypes.set_size       ( num_probesets, num_samples );
        db.likelihoods.copy_size    ( db.genotypes );
        db.posteriors.copy_size     ( db.genotypes );

        mutex_.reset(new std::mutex);
    }

    virtual void start(void) override
    {
        auto& logger = cpt::dbg;
        auto& db = this->mut_data_pool();
        auto& probeset_cube         = probeset_cube_        -> get();
        auto& results               = results_              -> get();
        auto& summary_of_probesets  = summary_of_probesets_ -> get();
        auto& allele_signals = probeset_cube;
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
                    for ( auto& e : probeset_cube.slice(i) ) e = log(e);
                    auto&& mdl = db.models[i];
                    auto&& obs = probeset_cube.slice(i);
                    #ifdef PROBESET_TRAINING_PLOT 
                    auto ps_name = probset_cube.z_axis.search_label_at(i);
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
        db.thread_pool->flush();
                
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
                results.push_entry(
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
            summary_of_probesets.push_entry(
                sample_vec[j]
              , num_calls[j] / total
            );
        }
    }

    virtual void finish(void) override
    {
        auto& db = this->mut_data_pool();
        auto& results = results_->get();
        auto& summary_of_probesets = summary_of_probesets_->get();
        mutex_.reset();

        // export genotype results
        {
            auto path = db.output_dir();
            path += "genotype.tsv";
            auto& node = db.require_output("genotype");
            db.add_output_path(node, path);

            std::ofstream os(path.string());
            results.ref()
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
            results_->release();
        }

        // export summary of probesets
        {
            auto path = db.output_dir();
            path += "call_rate.tsv";
            auto& node = db.require_output("call_rate");
            db.add_output_path(node, path);

            std::ofstream os(path.string());
            summary_of_probesets.ref()
                .select(
                    "sample_name"
                  , "call_rate"
                )
                .where([](const auto&){ return true; })
                .view()
                .dump(os);

            os.close();
            summary_of_probesets_->release();
        }
    }
};

template <>
class Genotyping<ca::AxiomGT, false>
    : public cpt::engine::NamedComponent
{
  public:
    using Base = engine::NamedComponent;
    using Base::Base;
    using Policy = cpt::algorithm::AxiomGT;
    using Tsvs = std::vector< cpt::format::Tsv<> >;
  
  public:
    std::size_t                  thread_num_                            ;

  private:
    int                          mdl_not_validate                       ;
    std::vector<std::string>     probeset_names                         ;
    std::vector<std::string>     sample_names                           ;  
    com_::ICPObjPtr< cf::Tsv<> > results_                               ;
    com_::ICPObjPtr< cf::Tsv<> > summary_of_probesets_                  ;
    com_::ICPObjPtr<
        cf::Cube<double>
    >                            probeset_cube_                         ;
    com_::ICPObjPtr<
        cf::Cube< double > 
    >                            specialsnps_info_cube_                 ;    
    com_::ICPObjPtr<
        cf::Cube< double > 
    >                            sample_specialsnps_cn_cube_            ;    
    std::unique_ptr< std::mutex> mutex_                                 ;
    std::set<std::string>        dbg_view_list_                         ;

  private:
    virtual void config_parameters(const bpt::ptree& p) override
    {
        using TypeAlias1 = decltype(results_)::element_type;
        
        mdl_not_validate = -1;  // Model not validate => unknown genotype => No call

        auto& db = this->mut_data_pool();
        auto json = cf::make_json( p );
        results_ = com_::require_w< cf::Tsv<> >(
            json, "results", com_::make_ref_parameter("Genotypes")
        );
        results_-> initialize(); 

        results_->get().register_column("probeset_name");
        results_->get().register_column("sample_name");
        results_->get().register_column("a_allele");
        results_->get().register_column("b_allele");
        results_->get().register_column("genotype");
        results_->get().register_column("posterior");

        probeset_cube_ = com_::require_w <cf::Cube<double>> ( 
              json
            , "probeset_cube"         
            ,  com_::make_ref_parameter("probeset_cube")  
        );
        specialsnps_info_cube_      = com_::require_w< cpt::format::Cube<double> > (
              json
            , "specialsnps_info_cube"
            ,  com_::make_ref_parameter("specialsnps_info_cube")
        );
        sample_specialsnps_cn_cube_ = com_::require_w< cf::Cube<double> > (
              json
            , "sample_special_snps_copynumbers_cube"
            ,  com_::make_ref_parameter("sample_special_snps_copynumbers_cube")
        );

        using TypeAlias2 = decltype(summary_of_probesets_)::element_type;
        summary_of_probesets_ = com_::require_w< cf::Tsv<> >(
              json, "summary_of_probesets"
            , com_::make_ref_parameter("SummaryOfProbesets")
        );
        summary_of_probesets_ -> initialize(); 
        summary_of_probesets_->get().register_column("sample_name");
        summary_of_probesets_->get().register_column("call_rate");

        thread_num_ = json.get_optional<std::size_t>("thread_num.content").value_or(16);

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
        probeset_cube_              ->  initialize();
        specialsnps_info_cube_      ->  initialize();
        sample_specialsnps_cn_cube_ ->  initialize();

        auto& probeset_cube              = probeset_cube_ -> get();
        auto num_samples                 = probeset_cube.n_cols;
        auto num_probesets               = probeset_cube.n_slices;
        probeset_names                   = probeset_cube.z_axis.get_labels();
        sample_names                     = probeset_cube.y_axis.get_labels();
        auto& specialsnps_info_cube      = specialsnps_info_cube_ -> get();
        db.axiom_gt_aux.set_copynumber_categories_for_each_probeset(
            specialsnps_info_cube, 
            probeset_names
        );
        auto& sample_specialsnps_cn_cube = sample_specialsnps_cn_cube_ -> get();
        db.axiom_gt_aux.set_copynumbers_from_sample_special_snps(
            sample_specialsnps_cn_cube, 
            probeset_names, 
            sample_names
        );
        db.require_axiomGT_models( db, probeset_names );
        db.igenotypes .set_size  ( num_probesets, num_samples );
        db.likelihoods.copy_size    ( db.igenotypes );
        db.posteriors .copy_size    ( db.igenotypes );
        db.thread_pool->resize_pool ( thread_num_   );

        mutex_.reset(new std::mutex);
    }

    virtual void start(void) override
    {
        auto& logger = cpt::dbg;
        auto& db = this->mut_data_pool();
        auto& probeset_cube         = probeset_cube_        -> get();
        auto& results               = results_              -> get();
        auto& summary_of_probesets  = summary_of_probesets_ -> get();
        auto& allele_signals     = probeset_cube;
        const auto num_probesets = allele_signals.n_slices;
        const auto num_samples   = allele_signals.n_cols;
        auto&  ocean             = db.ocean;
        auto&  maxconfidence     = db.maxconfidence;

        /* monitor setting */
        auto& monitor = db.monitor();
        constexpr static auto component_tag = "AxiomGT Calling";
        monitor.set_monitor( component_tag, 2 );
        monitor.log( component_tag, "Start ... " );

        const auto job_pack = std::ceil(static_cast<double>(num_probesets) / 20 /  db.thread_pool->get_thread_num());   


        monitor.set_monitor( "axiomGT::calling_process", std::ceil(num_probesets / job_pack ) + 2);
        monitor.log( "axiomGT::calling_process", "Start ... " );
        for (auto first = 0u; first < num_probesets; first += job_pack)
        {
            monitor.log( "axiomGT::calling_process", "Calling ... " );
            auto last = first + job_pack;
            if (last > num_probesets)
                last = num_probesets;

            db.thread_pool->job_post([&, first, last](void)
            {
                std::unique_lock<std::mutex> lock(*mutex_, std::defer_lock);

                arma::Row<int>          sgenotypes(num_samples);
                arma::Row<double>       likelihoods(num_samples);
                arma::Row<double>       posteriors(num_samples);

                for (auto i = first; i != last; ++i)
                {
                    auto&&             data             = probeset_cube.slice(i);
                    const auto&        dbmodel          = db.data_likelihood[i];
                    const auto&        dbcnfromspec     = db.axiom_gt_aux.copynumber_categories[i];
                    arma::irowvec      irow_copynumbers = db.axiom_gt_aux.copynumbers.row(i);

                    for(const auto& spec_copy_number: dbcnfromspec)
                    {
                        arma::uvec s_ids_by_spec_cn = arma::find( irow_copynumbers == spec_copy_number );
                        if( s_ids_by_spec_cn.size() == 0 ) continue;

                        const arma::mat&  obs = data.cols( s_ids_by_spec_cn        );
                        arma::Row<int>    cn_sgenotypes  ( s_ids_by_spec_cn.size() );
                        arma::Row<double> cn_likelihoods ( s_ids_by_spec_cn.size() );
                        arma::Row<double> cn_posteriors  ( s_ids_by_spec_cn.size() );

                        auto cn_model_it = dbmodel.find( spec_copy_number );
                        if( cn_model_it == dbmodel.end() || !cn_model_it->second.validate )
                        {
                            cn_sgenotypes .fill( mdl_not_validate );  // Model not validate => unknown genotype => No call
                            cn_likelihoods.fill( arma::datum::nan );
                            cn_posteriors .fill( arma::datum::nan );
                        }
                        else
                        {
                            auto& cn_model = cn_model_it->second.model;
                            Policy::infer(
                                cn_sgenotypes
                              , cn_likelihoods
                              , cn_posteriors
                              , obs
                              , cn_model
                              , spec_copy_number
                              , ocean
                              , maxconfidence
                            );
                        }

                        sgenotypes .elem( s_ids_by_spec_cn ) = cn_sgenotypes;
                        likelihoods.elem( s_ids_by_spec_cn ) = cn_likelihoods;
                        posteriors .elem( s_ids_by_spec_cn ) = cn_posteriors;
                    }


                    lock.lock();
                    db.igenotypes .row(i) = sgenotypes;
                    db.likelihoods.row(i) = likelihoods;
                    db.posteriors .row(i) = posteriors;
                    lock.unlock();
                }
            });
        }
        db.thread_pool->flush();
        monitor.log( "axiomGT::calling_process", "Done" );
        
        monitor.set_monitor( "axiomGT::results_summarization", num_probesets + 2);
        monitor.log( "axiomGT::results_summarization", "Start ... " );
        double total = 0.0;
        std::vector<double> num_calls( num_samples, 0.0 );
        probeset_names = allele_signals.z_axis.get_labels();
        sample_names   = allele_signals.y_axis.get_labels();
        for ( size_t i = 0; i != num_probesets; ++i )
        {
            monitor.log( "axiomGT::results_summarization", "Summarizing ... " );
            const auto& cn_models = db.data_likelihood[i];
            // if (flag)
                total += 1.0;

            for (size_t j = 0; j != num_samples; ++j)
            {
                auto cn_models_it = cn_models.find( db.axiom_gt_aux.copynumbers(i, j) );

                bool flag = cn_models_it->second.validate;
                results.push_entry(
                    probeset_names[i]
                  , sample_names[j]
                  , allele_signals(0, j, i)
                  , allele_signals(1, j, i)
                  , static_cast<int32_t>(flag? db.igenotypes(i, j): mdl_not_validate ) // Model not validate => unknown genotype => No call
                  , static_cast<double> (flag? db.posteriors(i, j): 0)
                );
                if (flag and db.posteriors(i, j) > 1-maxconfidence)
                    num_calls[j] += 1.0;
            }
        }
        monitor.log( "axiomGT::results_summarization", "Done ... " );

        const auto& sample_vec = allele_signals.y_axis.get_labels();

        for (size_t j = 0; j != num_samples; ++j)
        {
            summary_of_probesets.push_entry(
                sample_vec[j]
              , num_calls[j] / total
            );
        }
        monitor.log( component_tag, "Complete!!!" );
    }

    virtual void finish(void) override
    {
        auto& db = this->mut_data_pool();
        auto& results = results_->get();
        auto& summary_of_probesets = summary_of_probesets_->get();
        mutex_.reset();

        // export genotype results
        {
            auto path = db.output_dir();
            path += "genotype.tsv";
            auto& node = db.require_output("genotype");
            db.add_output_path(node, path);

            std::ofstream os(path.string());
            results.ref()
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
            results_->release();
        }

        // export summary of probesets
        {
            auto path = db.output_dir();
            path += "call_rate.tsv";
            auto& node = db.require_output("call_rate");
            db.add_output_path(node, path);

            std::ofstream os(path.string());
            summary_of_probesets.ref()
                .select(
                    "sample_name"
                  , "call_rate"
                )
                .where([](const auto&){ return true; })
                .view()
                .dump(os);

            os.close();
            summary_of_probesets_->release();
        }
    }
};

}
}
#else
#endif
