#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
#include <vector>
#include <CPT/format/tsv.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <CPT/utility/allele_signals_loader.hpp>
#include <CPT/engine/data_pool/shared_data_manager.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <CPT/algorithm/allele_min.hpp>
#include <CPT/algorithm/allele_max.hpp>
#include <CPT/algorithm/allele_avg.hpp>
#include <boost_addon/range_eval.hpp>
#include <boost_addon/range_sink.hpp>
#include <CPT/components/birdseed/typedef.hpp>
#include <CPT/algorithm/probeset_rnd_choice.hpp>
#include <CPT/algorithm/log_transform.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <Nucleona/container/vector.hpp>
#include <CPT/algorithm/g_m_m_fitting.hpp>
#include <complex>
#include <CPT/algorithm/argsort.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <CPT/algorithm/acc_count.hpp>
#include <CPT/algorithm/argmin.hpp>
#include <CPT/algorithm/set_diff.hpp>
#include <CPT/algorithm/distance.hpp>
#include <boost_addon/range_filter.hpp>
#include <boost_addon/range_vector.hpp>
#include <CPT/algorithm/matrix_range.hpp>
#include <CPT/algorithm/bic.hpp>
#include <CPT/algorithm/fld.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost_addon/range_indexed.hpp>
#include <CPT/algorithm/rm_same.hpp>
#include <CPT/algorithm/mdl_components.hpp>
#include <boost/range/irange.hpp>
#include <CPT/algorithm/posterior.hpp>
#include <boost_addon/range_vector.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <CPT/algorithm/runstats.hpp>
#include <boost_addon/range_eval.hpp>
#include <CPT/algorithm/g_m_m_util/access.hpp>
#include <CPT/algorithm/matrix_range/submatrix.hpp>
#include <boost_addon/flattened.hpp>

#ifndef NO_VTK
#include <CPT/visualization/geometrics/ellipse.hpp>
#include <CPT/visualization/table.hpp>
#include <CPT/visualization/scatter_plot.hpp>
#endif

#include <boost_addon/range_filter.hpp>
#include <CCD/utility/tune_scope.hpp>
#include <CPT/algorithm/g_m_m_util/p_r_fitting.hpp>
#include <CPT/algorithm/g_m_m_util/o_c_c_fitting.hpp>
#include <CPT/algorithm/g_m_m_util/abs_p_fitting.hpp>
#include <CPT/algorithm/g_m_m_util/dist_angle_sort.hpp>
#include <CPT/algorithm/paralleled.hpp>
#include <CPT/utility/gprofiler.hpp>
#include <CPT/utility.hpp>
#include <CPT/components/birdseed/dist_constrain.hpp>
#include <CPT/utility/thread_task_time_reporter.hpp>
#include <CPT/algorithm/g_m_m_util/g_m_m.hpp>
#include <CPT/algorithm/g_m_m_util/kmean.hpp>
#include <CPT/components/birdseed/kmeans_algo_factory.hpp>

extern "C" void openblas_set_num_threads( int thread_num );

namespace cpt {
namespace component {
namespace birdseed {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
namespace md = mlpack::distribution;
namespace mg = mlpack::gmm;
namespace bt = boost;
namespace cam = ca::matrix_range;
namespace cagu = ca::g_m_m_util;
namespace bad = boost_addon;
#ifndef NO_VTK
namespace cv = cpt::visualization;
namespace cvg = cpt::visualization::geometrics;
#endif
namespace probeset_training_detail {
    using ProbesetTrainingBase = engine::NamedComponent;
}
namespace com_ = cpt::engine::data_pool::component_object_manager;
using NPTimer = ::LightTuneScope<::LightTuneScopeNoPool>;
class ProbesetTraining 
: public probeset_training_detail::ProbesetTrainingBase
{
    static constexpr auto& tlog   = cpt::logout6;
    using GMM = mlpack::gmm::GMM;
    using Base          = probeset_training_detail::ProbesetTrainingBase;
    using DataPoolType  = typename Base::DataPoolType;
    using RndIds        = typename ca::ProbesetRndChoice::RndIds;
    using GMean         = arma::vec;
    using GMMean        = std::vector<GMean>;
    using SizeRow       = arma::Row<std::size_t>;
  public:
    static constexpr auto& logger = cpt::logout5;
  private:
    // template<class METRIC>
    // using DistConstrainT = DistConstrain<
    //     GMM, arma::mat, METRIC, decltype(logger)
    // >;
    // template<class METRIC>
    // using DistConstrainT = DistConstrainSimpleBic<
    //     GMM, arma::mat, METRIC, decltype(logger)
    // >;
    // template<class METRIC>
    // using DistConstrainT = DistConstrain2<
    //     GMM, arma::mat, METRIC, decltype(logger)
    // >;
    template<class METRIC>
    using DistConstrainT = DistConstrain2<
        GMM, arma::mat, METRIC, decltype(logger)
    >;
    using DistConstrainPtr = std::unique_ptr<IDistConstrain<
        GMM, arma::mat, decltype(logger)
    >>;
  public:
    std::size_t              select_probeset_num                    ;
    // ced::Shared<GMM>         grand_model                            ;
    com_::ICPObjPtr<GMM>     grand_model_                           ;
    com_::ICPObjPtr<
        cf::Cube<double>
    >                        probeset_cube_                         ;
    std::size_t              thread_num_                            ;
    cagu::FittingAlgoPtr     fitting_algo                           ;
    DistConstrainPtr         dist_constrain                         ;
    std::set<std::string>    dbg_view_list                          ;
    std::string              archive                                ;
    std::string              fitting_algo_name                      ;
    bpt::ptree               algo_params                            ;

    template< class... T >
    ProbesetTraining(
          const DataPoolType& data_pool
        , const T&... o
    )
    : Base          ( data_pool, o... )
    {}
    void parameter_check()
    {
#ifdef DEBUG_USE_CACHE
        auto& db( this->mut_data_pool() );
        /* 
         * if parameter ready then just use
         * if parameter no ready then try use cache
         * if cache miss then fail
         *
         */
        // const static std::string cache_path = "./probeset_training.archive";
        if ( archive != "" )
        {
            if(bt::filesystem::exists(archive))
            {
                std::ifstream fin(archive)           ;
                bt::archive::text_iarchive ia ( fin )   ;
                throw std::logic_error ( "not implementation error" );
                // ia & grand_model.instance->d            ;
                // ia & db.cube                            ;
            }
            else
            {
                std::ofstream fout ( archive )       ;
                bt::archive::text_oarchive oa ( fout )  ;
                throw std::logic_error ( "not implementation error" );
                // oa & grand_model.instance->d            ;
                // oa & db.cube                            ;
            }
        }
#endif 
    }
    
    auto get_gmm_dist_constrain()
    {
        // return new DistConstrainT<cadc::GMM>(logger);
        return new DistConstrainT<cadc::GMM>(5, 10, logger);
        // return new DistConstrainT<cadc::GMM>(4, 10, grand_model.get(), logger);
    }

    template<class T>
    void algo_config( const cf::Json<T>& algo )
    {
        /* create fitting algorithm */
        auto jfitting_algo_name = algo.template get<std::string>("name");
        fitting_algo_name = jfitting_algo_name;
        if ( false ) {}
        else if ( jfitting_algo_name == "PRFitting" ) 
        { 
            fitting_algo.reset      ( new cagu::PRFitting( algo ) );
            dist_constrain.reset    ( get_gmm_dist_constrain() );
            // return cagu::FittingAlgoPtr( new cagu::PRFitting( algo ) );
        }
        else if ( jfitting_algo_name == "AbsPFitting" )
        {
            fitting_algo.reset      ( new cagu::AbsPFitting( algo ) );
            dist_constrain.reset    ( get_gmm_dist_constrain() );
        }
        else if ( jfitting_algo_name == "OCCFitting" )
        {
            fitting_algo.reset      ( new cagu::OCCFitting( algo ) );
            dist_constrain.reset    ( get_gmm_dist_constrain() );
        }
        else if ( jfitting_algo_name == "PRFittingRT" )
        {
            auto subtype = algo.template get<std::string>("subtype");
            if ( subtype == "GMM" )
            {
                fitting_algo.reset      ( new cagu::PRFittingRT<cagu::GMMTrain, cagu::GMMClassify>( algo ) );
                dist_constrain.reset    ( get_gmm_dist_constrain() );
            }
            else if ( subtype == "KMeans" )
            {
                KMeansAlgoFactory<
                      cagu::PRFittingRT
                    , DistConstrainT
                >::set_component(*this, algo );
            }
            else
            {
                throw std::logic_error("unknow PRFitting subtype : " + subtype );
            }
        }
        else if ( jfitting_algo_name == "AbsPFittingRT" )
        {
            auto subtype = algo.template get<std::string>("subtype");
            if ( subtype == "GMM" )
            {
                fitting_algo.reset      ( new cagu::AbsPFittingRT<cagu::GMMTrain, cagu::GMMClassify>( algo ) );
                dist_constrain.reset    ( get_gmm_dist_constrain() );
            }
            else if ( subtype == "KMeans" )
            {
                KMeansAlgoFactory<
                      cagu::PRFittingRT
                    , DistConstrainT
                >::set_component(*this, algo );
            }
            else
            {
                throw std::logic_error("unknow AbsPFitting subtype : " + subtype );
            }
        }
        else if ( jfitting_algo_name == "OCCFittingRT" )
        {
            auto subtype = algo.template get<std::string>("subtype");
            if ( subtype == "GMM" )
            {
                fitting_algo.reset      ( new cagu::OCCFittingRT<cagu::GMMTrain, cagu::GMMClassify>( algo ) );
                dist_constrain.reset    ( get_gmm_dist_constrain() );
            }
            else if ( subtype == "KMeans" )
            {
                KMeansAlgoFactory<
                      cagu::PRFittingRT
                    , DistConstrainT
                >::set_component(*this, algo );
            }
            else
            {
                throw std::logic_error("unknow AbsPFitting subtype : " + subtype );
            }
        }
        else
        {
            throw std::logic_error("unknow fitting algorithm : " + jfitting_algo_name);
        }
    }
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        /* TODO require cube */

        auto&  db ( this->mut_data_pool() );
        auto json ( cf::make_json( p ) );

        select_probeset_num = json
            .get_optional<std::size_t>("select_probeset_num.content")
            .value_or(0);

        grand_model_ = com_::require_w <GMM> ( 
              json
            , "grand_model"         
            ,  com_::make_ref_parameter("GrandModel")  
        );
        // db.require( grand_model , "GrandModel" );
        probeset_cube_ = com_::require_w <cf::Cube<double>> ( 
              json
            , "probeset_cube"         
            ,  com_::make_ref_parameter("probeset_cube")  
        );

        algo_params = json.get_child("fitting_algo.content").root;

        archive      = json
            .get_optional<std::string>("archive.content")
            .value_or("");
        cpt::dbg << "archive : " << archive << std::endl;
        thread_num_ = json.get_optional<std::size_t>("thread_num.content").value_or(16);
        auto dvlp = json.get_optional<std::string>("dbg_view_list.content");
        if ( dvlp )
        {
            cpt::dbg << "dbg_view_list : " << dvlp.get() << std::endl;
            for ( auto&& line : std::ifstream(dvlp.get()) | ca::getlined() )
            {
                dbg_view_list.emplace(line);
            }
        }
    }
    virtual void initialize() override
    {
        /* TODO load cube       */
        parameter_check();
        auto& db( this->mut_data_pool() );
        probeset_cube_  ->  initialize();
        grand_model_    ->  initialize();
        auto& probeset_cube = probeset_cube_->get();
        db.models.resize(probeset_cube.n_slices);
        db.validities.resize(probeset_cube.n_slices, false);
        algo_config ( cf::make_json( algo_params ) );
        db.thread_pool->resize_pool( thread_num_ );
    }
    template<class VEC>
    auto permute_dist_by_arg( const GMM& m, VEC& vec)
    {
        std::vector<md::GaussianDistribution> newdist;
        arma::vec weight(vec.size());
        for ( auto&& e : vec | glue(ba::indexed(0)) )
        {
            weight[e.index()] = m.Weights()[e.value()];
            newdist.emplace_back(m.Component(e.value()));
        }
        return GMM(newdist, weight);
    }
    template<class I1, class I2>
    auto cyci(I1 i1, I2 i2 )
    {
        while ( i1 < 0 ) i1 += i2;
        return i1 % i2;
    }
    std::map<int,int> norm_component_pos( const GMM& model, const GMM& ref )
    {
        auto I = model.Gaussians();
        auto J = ref.Gaussians();
        auto prob = model
            | ca::mdl_components()
            | glue(ba::transformed (
                [](auto&& comp)->auto&{ return comp.Mean(); }
            ))
            | ca::posteriors_range( ref )
        ;
        auto pi = arma::Mat<std::size_t>(I, J, arma::fill::zeros);
        auto score = arma::mat(I + 1, J + 1, arma::fill::zeros);
        for ( auto i : bt::irange(std::size_t(0), I) )
        {
            for ( auto j : bt::irange(i, i + J - I + 1 ) )
            {
                auto best = score(
                      cyci(i - 1, I + 1)
                    , cyci(j - 1, J + 1)
                ) + prob[i][j];
                pi(i, j) = 1;
                if ( best < score(i, cyci(j - 1, J + 1)) )
                {
                    best = score(i, cyci(j - 1, J + 1));
                    pi(i, j) = 0;
                }
                score(i, j) = best;
            }
        }
        int i = I - 1;
        int j = J - 1;
        std::map<int, int> res;
        while ( i >= 0 and j >= 0 )
        {
            if( pi(i, j) == 1)
            {
                res[i] = j;
                i --;
            }
            j --;
        }
        return res;
    }
    template<class I>
    auto infer_missing_clusters( 
          ca::RunStats& ss1
        , ca::RunStats& ss2
        , GMM& mdl
        , const std::vector<I>& cases 
    ) 
    {
        // TODO check position mapping
        auto ss1_all_inds = cu::irange_0(6);
        auto ss2_all_inds = cu::irange_0(9);
        std::map<int, std::vector<int>> ss1_m_inds_map {
              { 0, { 0, 1 } }
            , { 1, { 2, 3 } }
            , { 2, { 4, 5 } }
        };
        std::map<int, std::vector<int>> ss2_m_inds_map {
              { 0, { 0, 1, 2 } }
            , { 1, { 3, 4, 5 } }
            , { 2, { 6, 7, 8 } }
        };
        auto m = ss1.mean;
        auto C = ss1.covariance();
        auto a = ss1_m_inds_map | ca::idx_access ( cases ) | ca::state_flattened();
        auto b = ss1_all_inds | ca::set_diff( a );
        auto temp_mean = mdl 
            | cagu::meaned()
            | ca::state_flattened()
            | ca::to_arma_vec()
        ;
        auto cond_mean = (
              ( m | ca::idx_access(a) | ca::to_arma_vec() )
            + ( (
                  ( ( temp_mean - (m | ca::idx_access(b) | ca::to_arma_vec())) | ca::arma_trans() )
                * arma::solve(
                      C | cam::submatd(b, b) | cam::submat_to_arma_mat()
                    , C | cam::submatd(b, a) | cam::submat_to_arma_mat()
                )
            ) | ca::arma_trans() )
        ).eval();

        m = ss2.mean;
        C = ss2.covariance();
        a = ss2_m_inds_map | ca::idx_access ( cases ) | ca::state_flattened();
        b = ss2_all_inds | ca::set_diff( a );

        auto temp_covariance( 
            mdl 
            | cagu::covarianced() 
            | ba::transformed(
                [] ( auto&& p )
                { 
                    return std::vector<double> { 
                          p(0, 0)
                        , p(0, 1)
                        , p(1, 1) 
                    }; 
                }
            )
            | ca::state_flattened()
            | ca::to_arma_vec()
        );
        auto cond_covariance = (
              ( m | ca::idx_access(a) | ca::to_arma_vec() )
            + ( (
                  ( ( temp_covariance - (m | ca::idx_access(b) | ca::to_arma_vec()) ) | ca::arma_trans() )
                * arma::solve(
                      C | cam::submatd(b,b) | cam::submat_to_arma_mat()
                    , C | cam::submatd(b,a) | cam::submat_to_arma_mat()
                )
            ) | ca::arma_trans() )
        ).eval()
        ;
        // reshape
        std::vector<arma::mat> cond_covariances;
        std::vector<arma::vec>       cond_means;
        for ( int i (0); i < cases.size(); i ++ )
        {
            auto step_cc = i * 3;
            arma::mat cc_rs = { 
                  { cond_covariance[0 + step_cc], cond_covariance[1 + step_cc] }
                , { cond_covariance[1 + step_cc], cond_covariance[2 + step_cc] }
            };
            arma::vec eigval;
            arma::mat eigvec;
            arma::eig_sym(eigval, eigvec, cc_rs);

            bool negative(false);
            logger << eigval << std::endl;
            for ( auto&& v : eigval )
            {
                if ( v < 0 ) 
                {
                    negative = true;
                    v = 1e-6;
                }
            }
            if ( negative )
            {
                logger << "negtive covariance detect" << std::endl;
                logger << eigval << std::endl;
                logger << eigvec << std::endl;
                cc_rs = eigvec;
                cc_rs.each_col(
                    [&eigval] ( arma::vec& col ) { col %= eigval; }
                );
                cc_rs = cc_rs * arma::trans( eigvec );
                logger << cc_rs << std::endl;
            }
            else
            {
                logger << "positive covariance detect" << std::endl;
                logger << cc_rs << std::endl;
            }
            auto step_cm = i * 2;
            arma::vec cm_rs = { cond_mean[0 + step_cm], cond_mean[1 + step_cm] };
            cond_means.emplace_back(std::move(cm_rs));
            cond_covariances.emplace_back ( std::move(cc_rs));
        }
        return std_addon::make_tuple(
              std::move(cond_means       )
            , std::move(cond_covariances )
        );
    }
    template<class MOD, class SMP>
    SizeRow classify(MOD&& mod, SMP&& smp)
    {
        SizeRow labels(smp.n_cols);
        mod.Classify(smp, labels);
        return labels;
    }
    virtual void NOINLINE start() override
    {
        // NPTimer timer("probeset_training");
        cu::GProfiler profiler ( "probeset_training.prof" );

        auto& db ( this->mut_data_pool() );
        auto& probeset_cube = probeset_cube_->get();

        // auto&& time_reporter = db;
        // auto probeset_training_rh ( time_reporter.get_holder(
        //       "probeset_training"
        //     , cu::ThreadDurationLabel::iterative_mode
        // ) );
        const auto& cube  ( probeset_cube );
        const ca::ProbesetRndChoice prc{};
        const ca::LogTransform lt{};
        const ca::GMMFitting g_m_m_fitting{};
        const auto cands ( prc.get_rnd_vec( probeset_cube, select_probeset_num ) );
        const std::size_t pack_size = 1000;
        ca::RunStats ss1 { 6 };
        std::mutex ss1_mux;
        ca::RunStats ss2 { 9 };
        std::mutex ss2_mux;
        std::vector<std::tuple<std::size_t, std::vector<std::size_t>>> missing_cluster;
        std::mutex missing_cluster_mux;
        std::mutex msg_mux;

        #ifdef PROBESET_TRAINING_PLOT
        std::vector<arma::mat>  missing_cluster_ob  ; // TODO remove me
        std::vector<double>     missing_cluster_bic ; // TODO remove me
        std::vector<double>     missing_cluster_fan ;
        std::map<std::size_t, std::vector<
            std::tuple<GMM, double, arma::mat>
        >> all_mdl; /* mid -> model, bic, ob TODO remove me */
        std::mutex all_mdl_mux; /* TODO remove me */
        #endif

        // LabelIdx&   probeset_idx_v ( probeset_idx.get() );

        /* monitor setting */
        constexpr static auto component_tag = "Component Birdseed Probeset Training";
        constexpr static auto probset_progress_tag = "Probeset Progress";
        auto& monitor = db.monitor();
        std::mutex monitor_mux;
        monitor.set_monitor(component_tag, 2);
        // monitor.set_monitor(probset_progress_tag, cands.size());

        monitor.log(component_tag, "training...");

        monitor.set_monitor( "probeset_training::model_train", 1, true );
        monitor.set_monitor( "model_train::fitting_algo_name", 1, true );
        monitor.set_monitor( "model_train::angle_sort", 1, true );
        monitor.set_monitor( "model_train::make_score", 1, true );
        monitor.set_monitor( "model_train::best_score", 1, true );
        monitor.set_monitor( "model_train::component_position_remap", 1, true );
        monitor.set_monitor( "running_stat::push_data", 1, true );
        monitor.set_monitor( "running_stat::compute", 1, true );

        auto non_parallel = [
              &ss1_mux, &ss2_mux
            , &missing_cluster_mux
            , &monitor_mux
            , &monitor
            // , &time_reporter
        ](auto&& pack)
        {
            monitor.thread_monitor_start( "running_stat::push_data" );

            // auto running_stat_rh ( time_reporter.get_holder(
            //       "running_stat::push_data"
            //     , cu::ThreadDurationLabel::iterative_mode
            // ) );
            cpt::dbg << "handle non_parallel code" << std::endl;
            std::vector<std::size_t> case3;
            std::vector<std::size_t> casen3;
            for ( auto&& tupi : pack | ::range_indexed(0))
            {
                auto&& tup = tupi.value();
                cpt::dbg << "is computable data : " << std::get<2>(tup) << std::endl;
                if ( std::get<2>(tup) )
                {
                    cpt::dbg << "Gaussian num : " << std::get<1>(tup) << std::endl;
                    if ( std::get<1>(tup) == 3 )
                    {
                        case3.emplace_back(tupi.index());
                    }
                    else
                    {
                        casen3.emplace_back(tupi.index());
                    }
                } 
            }
            if ( case3.size() > 0 )
            {
                ss1_mux.lock();
                ss2_mux.lock();
                for( auto&& i : case3 )
                {
                    std::get<0>(pack.at(i))();
                }
                ss2_mux.unlock();
                ss1_mux.unlock();
            }
            if ( casen3.size() > 0 )
            {
                missing_cluster_mux.lock();
                for( auto&& i : casen3 )
                {
                    std::get<0>(pack.at(i))();
                }
                missing_cluster_mux.unlock();
            }
            {
                std::lock_guard<std::mutex> lock(monitor_mux);
                for ( auto&& t : pack )
                {
                    cpt::dbg << "monitor call pstrain" << std::endl;
                    // monitor.log(probset_progress_tag, "processing...");
                }
            }

            monitor.thread_monitor_end( "running_stat::push_data" );
        };

        cands 
        | ba::transformed(
            [
                  &prc
                , &lt
                , &g_m_m_fitting
                , &ss1, &ss1_mux
                , &ss2, &ss2_mux
                , &missing_cluster, &missing_cluster_mux
                , this
                , &db
                , &cube
                , &probeset_cube
                // , &msg_mux
                , &monitor, &monitor_mux
                // , &time_reporter
                #ifdef PROBESET_TRAINING_PLOT
                , &all_mdl, &all_mdl_mux
                , &missing_cluster_ob
                , &missing_cluster_bic
                , &missing_cluster_fan
                #endif
            ]( auto mid )
            {
                monitor.thread_monitor_start( "probeset_training::model_train" );

                // auto model_train_rh ( time_reporter.get_holder(
                //       "probeset_training::model_train"
                //     , cu::ThreadDurationLabel::parallel_mode
                // ) );
                std::tuple<
                      std::function<void(void)>
                    , int
                    , bool
                > non_parallel_data;
                auto& dbmodel = db.models[mid];
                auto& allele_sample ( cube.slice(mid) );
                arma::vec min(2);
                arma::vec max(2);
                arma::vec avg(2);
                lt.get_min_max_avg( /* min max avg, bad call name*/ 
                      allele_sample
                    , min, max, avg
                    , cube
                );
                const auto& grand_model( grand_model_->get() );
                std::vector<GMMean> g_m_means  {
                      GMMean {
                          grand_model.Component(0).Mean()
                      }
                    , GMMean {
                          grand_model.Component(1).Mean()
                      }
                    , GMMean {
                          grand_model.Component(2).Mean()
                      }
                    , GMMean {
                          grand_model.Component(0).Mean()
                        , grand_model.Component(2).Mean()
                      }
                    , GMMean {
                          grand_model.Component(0).Mean()
                        , grand_model.Component(1).Mean()
                      }
                    , GMMean {
                          grand_model.Component(1).Mean()
                        , grand_model.Component(2).Mean()
                      }
                    , GMMean {
                          grand_model.Component(0).Mean()
                        , grand_model.Component(1).Mean()
                        , grand_model.Component(2).Mean()
                      }
                    , GMMean { 
                          GMean { avg[0], avg[1] }
                      }
                    , GMMean {
                          GMean { max[0], min[1] }
                        , GMean { min[0], max[1] }
                    }
                    , GMMean {
                          GMean { max[0], min[1] }
                        , GMean { avg[0], avg[1] }
                        , GMean { min[0], max[1] }
                    }
                };
                ca::ArgSort argsort;
                // DistConstrain<GMM, arma::mat> constrain( 
                //     5, g_m_means.size(), grand_model, logger
                // );
                // DistConstrainSimple<GMM, arma::mat> constrain( logger );
                auto constrain = dist_constrain->clone();
                for ( auto& g_m_mean : g_m_means )
                {
                    monitor.thread_monitor_start( "model_train::fitting_algo_name" );

                    // auto one_initial_model_rh ( time_reporter.get_holder(
                    //       "model_train::" + fitting_algo_name
                    //     , cu::ThreadDurationLabel::parallel_mode
                    // ) );
                    // fitting and remove outlier
                    logger << "initial model" << std::endl;
                    this->show_mean_info ( g_m_mean , logger);
                    logger << std::endl;
                    arma::mat ob = allele_sample;
                    try {
                        GMM model = fitting_algo->operator()( 
                            ob, g_m_mean
                        );
                        {
                            monitor.thread_monitor_start( "model_train::angle_sort" );

                            // auto angle_sort_rh ( time_reporter.get_holder(
                            //       "model_train::angle_sort"
                            //     , cu::ThreadDurationLabel::parallel_mode
                            // ) );
                            model = cagu::dist_angle_sort( model );

                            monitor.thread_monitor_end( "model_train::angle_sort" );
                        }

                        if ( ob.n_cols < 1 ) 
                            continue;
                        {
                            monitor.thread_monitor_start( "model_train::make_score" );

                            // auto scoring_rh ( time_reporter.get_holder(
                            //       "model_train::make_score"
                            //     , cu::ThreadDurationLabel::parallel_mode
                            // ) );
                            constrain->score( model, ob, logger);

                            monitor.thread_monitor_end( "model_train::make_score" );
                        }
                        {
                            // #ifdef PROBESET_TRAINING_PLOT
                            // std::lock_guard<std::mutex> lock ( all_mdl_mux );
                            // all_mdl[mid].emplace_back(
                            //     model, model_bic, ob
                            // );
                            // #endif
                        }
                    } catch ( ... ) {}

                    monitor.thread_monitor_end( "model_train::fitting_algo_name" );
                }

                monitor.thread_monitor_start( "model_train::best_score" );

                // auto best_score_rh ( time_reporter.get_holder(
                //       "model_train::best_score"
                //     , cu::ThreadDurationLabel::parallel_mode
                // ) );
                auto constrain_res ( constrain->get_best(logger) );
                // best_score_rh.release();

                monitor.thread_monitor_end( "model_train::best_score" );

                auto& constrain_dat ( std::get<1>(constrain_res) );
                
                auto& best_mdl ( constrain_dat.mdl );
                if ( !std::get<0>(constrain_res) ) 
                {
                    logger << "fail model ! " << std::endl;
                    db.validities[mid] = false;
                    std::get<2>(non_parallel_data) = false;

                    monitor.thread_monitor_end( "probeset_training::model_train" );

                    return non_parallel_data;
                }
                else 
                {
                    logger << "find pass model !" << std::endl;
                }
                /* mapper start we dont know the correct component position if threre are some missing cluster. */

                monitor.thread_monitor_start( "model_train::component_position_remap" );

                // auto component_position_remap_rh ( time_reporter.get_holder(
                //       "model_train::component_position_remap"
                //     , cu::ThreadDurationLabel::parallel_mode
                // ) );
                auto mapper ( this->norm_component_pos ( best_mdl, grand_model ) );
                // component_position_remap_rh.release();

                monitor.thread_monitor_end( "model_train::component_position_remap" );
                monitor.thread_monitor_start( "running_stat::compute" );

                // auto running_stat_rh ( time_reporter.get_holder(
                //       "running_stat::compute"
                //     , cu::ThreadDurationLabel::parallel_mode
                // ) );
                std::get<1>( non_parallel_data ) = best_mdl.Gaussians();
                if ( best_mdl.Gaussians() == 3 )
                {
                    auto ss1v = best_mdl | cagu::meaned() 
                        | ca::state_flattened() | ca::to_arma_vec();
                    auto ss2e = best_mdl
                        | cagu::covarianced()
                        | ::pipe_redirect(
                            []( auto&& cov_rng )
                            {
                                arma::vec res (9);
                                for ( auto&& cov_p : cov_rng | ::range_indexed(0))
                                {
                                    res(3*cov_p.index()    ) = cov_p.value()(0,0);
                                    res(3*cov_p.index() + 1) = cov_p.value()(0,1);
                                    res(3*cov_p.index() + 2) = cov_p.value()(1,1);
                                }
                                return res;
                            }
                        )
                    ;
                    std::get<0>( non_parallel_data ) = 
                        [
                              ss2es = std::move(ss2e), &ss2
                            , ss1vs = std::move(ss1v), &ss1
                            , mids = ::vcopy(mid)
                        ]()
                        {
                            ss2.push( ss2es );
                            ss1.push( ss1vs );
                            cpt::dbg << "mid : " << mids << " is 3 comp ( good ) " << std::endl;
                        };
                    db.validities[mid] = true;
                    logger << "mid : " << mid << " is a valid model" << std::endl;
                    // { /* debug */
                    //     auto labels( this->classify( best_mdl, allele_sample ) );
                    // }
                }
                else
                {
                    
                    auto mcv = (
                        cu::irange_0(std::size_t(3))
                        | ba::filtered(
                            [&mapper] ( auto c )
                            {
                                for( auto& p : mapper )
                                    if ( c == p.second ) return false;
                                return true;
                            }
                        )
                        | ::to_vector
                    );
                    std::get<0>( non_parallel_data ) = [
                          &missing_cluster
                        , mcvs = std::move( mcv )
                        , mids = ::vcopy(mid)
                        #ifdef PROBESET_TRAINING_PLOT
                        , constrain_dat
                        , missing_cluster_ob
                        , missing_cluster_bic
                        , missing_cluster_fan
                        #endif
                    ]() 
                    #ifdef PROBESET_TRAINING_PLOT
                    mutable
                    #endif
                    {
                        cpt::dbg << "missing cluster mid : " << mids << std::endl;
                        missing_cluster.emplace_back(mids, mcvs);
                        #ifdef PROBESET_TRAINING_PLOT
                        missing_cluster_ob.emplace_back(constrain_dat.ob);
                        missing_cluster_bic.emplace_back(constrain_dat.bic);
                        missing_cluster_fan.emplace_back(constrain_dat.fan);
                        #endif
                    };
                }

                monitor.thread_monitor_end( "running_stat::compute" );

                // running_stat_rh.release();
                dbmodel = best_mdl;
                {
                    // TODO std::lock_guard<std::mutex> lock(monitor_mux);
                    // TODO monitor.log(probset_progress_tag, "processing...");
                    #ifdef PROBESET_TRAINING_PLOT
                    std::lock_guard<std::mutex> lock(monitor_mux);
                    auto ps_name = probeset_cube.z_axis.search_label_at(mid);
                    if ( dbg_view_list.find(ps_name) != dbg_view_list.end() )
                    {
                        logger << "on training best model : " << ps_name << std::endl;
                        this->show_detail_train_status( 
                            std::ofstream(ps_name + "_train_best.txt"), best_mdl, constrain_dat.bic, constrain_dat.fan
                            // , all_mdl.at(mid)
                            , allele_sample, constrain_dat.ob, ps_name + "_train_best.png"
                        );
                        auto labels( this->classify( best_mdl, allele_sample ) );
                        logger << labels << std::endl;
                        this->show_plot_sample( allele_sample, labels, ps_name + "_train_smp.png" );
                    }
                    #endif
                }
                std::get<2>(non_parallel_data) = true;

                monitor.thread_monitor_end( "probeset_training::model_train" );

                return non_parallel_data;
            }
        ) 
        | ca::parallel_eval2( 
            *(db.thread_pool)
            , cands.size() / 20 / db.thread_pool->get_thread_num() 
            , non_parallel
        );

        monitor.set_monitor("Missing Cluster Imputation", missing_cluster.size() +2);
        monitor.log("Missing Cluster Imputation", "Start");

        logger << "total missing : " << missing_cluster.size() << std::endl;
        logger << "total valid model ( phase 1 ): " << valid_model_num() << std::endl;
        logger << "total invalid model ( phase 1 ): " << invalid_model_num() << std::endl;

        for( auto&& tup_i : missing_cluster | ::range_indexed(0))
        {
            // auto model_imputation_rh ( time_reporter.get_holder(
            //       "model_imputation"
            //     , cu::ThreadDurationLabel::iterative_mode
            // ) );
            auto& mid ( std::get<0>(tup_i.value()) );
            auto& m(db.models.at(mid));
            auto& cases = std::get<1>(tup_i.value());
            auto& s = probeset_cube.slice(mid);
            #ifdef PROBESET_TRAINING_PLOT
            auto& mob  = missing_cluster_ob.at(tup_i.index());
            auto& mbic = missing_cluster_bic.at(tup_i.index());
            auto& mfan = missing_cluster_fan.at(tup_i.index());
            #endif 
            if ( ss1.N > 40 )
            {
                #ifdef PROBESET_TRAINING_PLOT
                auto ps_name = probeset_cube.z_axis.search_label_at(mid);
                auto show = dbg_view_list.find(ps_name) != dbg_view_list.end();
                if ( show )
                {
                    logger << "before infer cluster : " << ps_name << std::endl;
                    this->show_detail_train_status( 
                        std::ofstream(ps_name + "_mdl_imp_before.txt")
                        , m, mbic, mfan
                        // , all_mdl.at(mid)
                        , s, mob, ps_name + "_mdl_imp_before.png"
                    );
                }
                #endif 
                auto cond = infer_missing_clusters( 
                      ss1
                    , ss2
                    , m
                    , cases
                );
                std::vector<md::GaussianDistribution> new_dists = m 
                    | ca::mdl_components() | to_vector;
                for ( auto&& c : cases | ::range_indexed(0))
                {
                    /* cases should be sorted */
                    // TODO check cases dependency
                    new_dists.insert(
                          new_dists.begin() + c.value()
                        , md::GaussianDistribution(
                              std::get<0>(cond)[c.index()]
                            , std::get<1>(cond)[c.index()]
                        )
                    );
                }
                arma::vec new_weights { 1.0/3, 1.0/3, 1.0/3 };
                m = GMM( new_dists, new_weights);
                db.validities[mid] = true;
                #ifdef PROBESET_TRAINING_PLOT
                if ( show )
                {
                    logger << "model imputation : " << ps_name << std::endl;
                    this->show_detail_train_status( std::ofstream(ps_name + "_mdl_imp_after.txt"), m, s, mob, ps_name + "_mdl_imp_after.png" );
                }
                #endif 
            }
            
            monitor.log("Missing Cluster Imputation", "processing...");
        }

        monitor.log("Missing Cluster Imputation", "Done");

        logger << "total valid model ( phase 2 ): " << valid_model_num() << std::endl;
        logger << "total invalid model ( phase 2 ): " << invalid_model_num() << std::endl;
        // { /* debug */
        //     auto&& logger = cpt::logout5;
        //     for ( auto&& mid : cands)
        //     {
        //         // logger << "model id : " << mid << std::endl;
        //         // logger << "ps name  : " << probeset_idx_v.at(mid) << std::endl;
        //         auto& m = db.models[mid];
        //         auto& allele_sample ( cube.slice(mid) );
        //         if ( db.validities.at(mid) )
        //         {
        //             // modelpsout << probeset_idx_v.at(mid) << '\n'; /* fine code */

        //             auto labels ( classify ( m, allele_sample ) );
        //             logger << "model check" << std::endl;
        //             // show_model_info(m, logger);
        //             // cv::ScatterPlot view;
        //             // view.show_legend = false;
        //             // view.initialize(); 
        //             // show_plot_sample( view, allele_sample, labels );
        //             // show_plot_model ( view, m );
        //             // view.render_and_start();
        //         }
        //         else
        //         {
        //             // logger << "fail model : " << probeset_idx_v.at(mid) << std::endl;
        //         }
        //     }
        //     modelpsout.flush(); /* fine code */
        // }
        monitor.log( "probeset_training::model_train", "Done", true );
        monitor.log( "model_train::fitting_algo_name", "Done", true );
        monitor.log( "model_train::angle_sort", "Done", true );
        monitor.log( "model_train::make_score", "Done", true );
        monitor.log( "model_train::best_score", "Done", true );
        monitor.log( "model_train::component_position_remap", "Done", true );
        monitor.log( "running_stat::push_data", "Done", true );
        monitor.log( "running_stat::compute", "Done", true );
        monitor.log( component_tag, "Done" );

    }
    std::size_t valid_model_num()
    {
        auto& db = this->mut_data_pool();
        std::size_t sum(0);
        for ( auto&& v : db.validities )
            sum+= ( v ? 1 : 0 );
        return sum;
    }
    std::size_t invalid_model_num()
    {
        auto& db = this->mut_data_pool();
        std::size_t sum(0);
        for ( auto&& v : db.validities )
            sum+= ( v ? 0 : 1 );
        return sum;
    }
    virtual void finish() override
    {
        auto& db = this->mut_data_pool();
        db.save_clustering_models(db);
        grand_model_->release();
        auto& node = db.require_output("model");
        db.add_output_path( 
              node
            , db.get_model_path(db)
        );
        // db.release( grand_model );
        // db.release( probeset_idx );
    }
    #ifndef NO_VTK
    template<class __OUT>
    void show_detail_train_status(
          __OUT                    &&  out
        , const GMM              &   mdl
        , const double           &   bic
        , const double           &   fan
        // , const std::vector<
        //     std::tuple<
        //         GMM, double
        //         , arma::mat 
        //     >
        // >                        &   candis_mdl
        , const arma::mat        &   sample
        , const arma::mat        &   sample_use_to_train
        , const std::string      &   pic_fname
    )
    {
        out << "choosed model" << std::endl;
        show_model_info(mdl, out);
        out << "bic : " << bic << std::endl;
        out << "fan : " << fan << std::endl;
        {
            cv::ScatterPlot view;
            view.show_legend = false;
            view.initialize(); 
            show_plot_sample( view, sample );
            show_plot_sample( view, sample_use_to_train, vtkPlotPoints::CROSS );
            show_plot_model ( view, mdl );
            view.image_dump(pic_fname);
            // view.render_and_start();
        }
        // for ( auto& ct : candis_mdl )
        // {
        //     auto&& cm       = std::get<0>(ct);
        //     auto&& cbic     = std::get<1>(ct);
        //     auto&& cob      = std::get<2>(ct);
        //     {
        //         out << "all ini model in this sample" << std::endl;
        //         show_model_info(cm, out);
        //         out << "bic : " << cbic << std::endl;
        //         cv::ScatterPlot cdview;
        //         cdview.show_legend = false;
        //         cdview.initialize(); 
        //         show_plot_sample( cdview, sample );
        //         show_plot_sample( cdview, cob, vtkPlotPoints::CROSS );
        //         show_plot_model ( cdview, cm );
        //         cdview.render_and_start();
        //     }
        // }
    }
    template<class __OUT>
    void show_detail_train_status(
          __OUT                    &&  out
        , const GMM              &   mdl
        , const arma::mat        &   sample
        , const arma::mat        &   sample_use_to_train
        , const std::string      &   pic_fname
    )
    {
        out << "choosed model" << std::endl;
        show_model_info(mdl, out);
        cv::ScatterPlot view;
        view.show_legend = false;
        view.initialize(); 
        show_plot_sample( view, sample );
        show_plot_sample( view, sample_use_to_train, vtkPlotPoints::CROSS );
        show_plot_model ( view, mdl );
        view.image_dump ( pic_fname );
        // view.render_and_start();
    }
    void show_plot_sample( 
          cv::ScatterPlot&              view
        , const arma::mat&              sample
        , const arma::Row<std::size_t>& labels 
        , int                           ps      = vtkPlotPoints::CIRCLE
    ) const 
    {
        auto table = cv::clustermatrix_to_vtk_table(sample, labels);
        std::vector<cv::ScatterPlot::PointConf> plot_confs {
            cv::ScatterPlot::PointConf {
                  ps
                , 255, 0, 0, 225              // color
                , 1.0                       // point width 
            }
            , cv::ScatterPlot::PointConf {
                  ps
                , 0, 255, 0, 225              // color
                , 1.0                       // point width 
            }
            , cv::ScatterPlot::PointConf {
                  ps
                , 0, 0, 255, 225              // color
                , 1.0                       // point width 
            }
        };
        for ( auto&& i : cu::irange_0(table.size()))
        {
            view.add_plot(table[i], 0, 1, plot_confs[i]);
        }
    }
    void show_plot_sample( 
        const arma::mat&              sample
        , const arma::Row<std::size_t>& labels 
        , const std::string         &   pic_fname
        , int                           ps      = vtkPlotPoints::CIRCLE
    ) const 
    {
        cv::ScatterPlot res;
        res.show_legend = false;
        res.initialize();
        show_plot_sample( res, sample, labels, ps );
        res.image_dump ( pic_fname );
        // res.render_and_start();
    }
    void show_plot_sample( 
          cv::ScatterPlot&              view
        , const arma::mat&              sample
        , int                           ps      = vtkPlotPoints::CIRCLE
    ) const 
    {
        auto table = cv::samplematrix_to_vtk_table(sample);
        std::vector<cv::ScatterPlot::PointConf> plot_confs {
            cv::ScatterPlot::PointConf {
                ps
                , 0, 0, 0, 225              // color
                , 1.0                       // point width 
            }
        };
        for ( auto&& i : cu::irange_0(table.size()))
        {
            view.add_plot(table[i], 0, 1, plot_confs[i]);
        }
    }
    void show_plot_model( 
          cv::ScatterPlot& view
        , const mg::GMM& gmm 
    ) const 
    {
        auto table = cv::model_to_vtktable( gmm );
        std::vector<cv::ScatterPlot::PlotConf> plot_confs {
            cv::ScatterPlot::PointConf {
                vtkPlotLine::NONE           // point style
                , 255, 0, 0, 225            // color
                , 3.0                       // point width 
            }
            , cv::ScatterPlot::PointConf {
                vtkPlotLine::NONE       // point style
                , 0, 255, 0, 225              // color
                , 3.0                       // point width 
            }
            , cv::ScatterPlot::PointConf {
                vtkPlotLine::NONE       // point style
                , 0, 0, 255, 225              // color
                , 3.0                       // point width 
            }
        };
        for ( auto&& i : cu::irange_0(table.size()))
        {
            view.add_line(table[i], 0, 1, plot_confs[i]);
        }
    }
    #else
    template<class __OUT>
    void show_detail_train_status(
          __OUT                  &&  out
        , const GMM              &   mdl
        , const double           &   bic
        , const double           &   fan
        , const arma::mat        &   sample
        , const arma::mat        &   sample_use_to_train
        , const std::string      &   pic_fname
    )
    {}
    template<class __OUT>
    void show_detail_train_status(
          __OUT                  &&  out
        , const GMM              &   mdl
        , const arma::mat        &   sample
        , const arma::mat        &   sample_use_to_train
        , const std::string      &   pic_fname
    )
    {}
    void show_plot_sample( 
          const arma::mat&              sample
        , const arma::Row<std::size_t>& labels 
        , const std::string         &   pic_fname
        , int                           ps      = 0
    ) const 
    {}
    #endif
    template<class OS = decltype(logger)&>
    void show_mean_info( const GMMean& g_m_mean, OS&& os = logger) const 
    {
        os << "   GMM component num : " << g_m_mean.size() << std::endl;
        os << "   GMM means : " << std::endl;
        for ( auto&& mean : g_m_mean )
        {
            os << "       " << mean[0] << ", " << mean[1] << std::endl;
        }
    }
    template<class OS = decltype(logger)& >
    void show_ob ( const arma::mat& ob, OS&& os = logger) const 
    {
        for ( auto& col : ob | glue(ca::col_range()) )
        {
            os << "(" << col[0] << ", " 
                << col[1] << ")" << std::endl;
        }
    }
    template<class OS = decltype(logger)& >
    void show_model_info( const GMM& gmm, OS&& os = logger ) const 
    {
        os << "gmm model info" << std::endl;
        gmm
            | ca::mdl_components()
            | glue( ::range_indexed(0) )
            | glue( ba::transformed ( 
                [&os] ( auto&& comp_i )
                {
                    os << "   sub_component " << comp_i.index() << std::endl;
                    os << "   mean : " 
                        << comp_i.value().Mean()[0] 
                        << ", "  << comp_i.value().Mean()[1] << std::endl;
                    os << "   cov : " << std::endl;
                    os << comp_i.value().Covariance();
                    os << std::endl;
                    return comp_i.value();
                }
            ))
            | ::endp
            ;
    }
};
}}}
#else
#endif 
