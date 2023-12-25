#pragma once 
#ifdef NEW_DATA_POOL
#include <CPT/engine/data_pool2.hpp>
#else
#include <memory>
#include <CPT/engine/data_pool/dummy_allele_signals.hpp>
#include <CPT/engine/data_pool/pipeline_schema.hpp>
#include <CPT/engine/data_pool/result_schema.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/engine/data_pool/mlpack_verbose.hpp>
// #include <CPT/engine/data_pool/signal_intansities.hpp>
#include <CPT/engine/data_pool/clustering_models.hpp>
#include <CPT/engine/data_pool/clustering_results.hpp>
#include <CPT/engine/data_pool/probeset_table.hpp>
#include <CPT/engine/data_pool/quantile.hpp>
#include <CPT/engine/data_pool/cube.hpp>
#include <CPT/engine/data_pool/sample_num.hpp>
#include <CPT/engine/data_pool/cdf.hpp>
#include <CPT/engine/data_pool/shared_object_manager.hpp>
#include <CPT/engine/data_pool/monitor.hpp>
#include <CPT/engine/data_pool/parallel_thread_pool.hpp>
#include <CPT/engine/data_pool/shared_data_manager.hpp>
#include <CPT/engine/data_pool/brlmmp.hpp>
// #include <CPT/engine/data_pool/cel.hpp>
#include <Nucleona/language.hpp>
#include <CPT/logger.hpp>
#include <fstream>
#include <iostream>
#include <CPT/utility/thread_task_time_reporter.hpp>
namespace cpt
{
namespace engine
{

auto& monitor_output_ = msg; //cpt::msg;
std::ofstream mointor_outfile_;
namespace cu = cpt::utility;
using namespace data_pool;
class DataPoolImpl 
// [Core]
: public PipelineSchema // input
, public ResultSchema   // output
, public DataPathsPool
, public MLPackVerbose
, public ParallelThreadPool
, public SharedObjectManager
, public SharedDataManager
, public cu::TickReport
, public cu::ThreadTaskTimeReporter
// [Opts]
// , public SignalIntansities
, public DummyAlleleSignals
, public ClusteringModels
, public ClusteringResults
, public ProbesetTable
, public Quantile
, public Cube
, public SampleNum
, public Cdf
, public BRLMMp
// , public Monitor< decltype( cpt::msg )>
// , public Cel
{
    Monitor< decltype( monitor_output_ )> monitor_;

  public:

    template<class OP>
    DataPoolImpl(OP& op)
    : PipelineSchema( op.pipeline_schema_stream_ )
    , ResultSchema  ( op.result_schema_stream_ )
    , ParallelThreadPool( 20 )
    // , MLPackVerbose ( )
    //
    , DummyAlleleSignals( *this )
    , ClusteringModels(*this)
    , ProbesetTable( *this )
    , Quantile( *this )
    , Cube( *this )
    , SampleNum( *this )
    , Cdf( *this )
    , BRLMMp( *this )
    , monitor_( monitor_output_, mointor_outfile_ )
    , SharedObjectManager()
    , cu::ThreadTaskTimeReporter( cpt::logout6, *this )
    , cu::TickReport( 3 )
    // , Cel            ( *this )
    {
        monitor_.set_output_dir( this->output_dir().string() );
        // this->tick_start();
    }

    auto& monitor()
    {
        return monitor_;
    }

//    DataPoolImpl(void)
//    {}

    DEFAULT_MOVE(DataPoolImpl);
    DISABLE_COPY(DataPoolImpl);
};
using DataPool = DataPoolImpl;
using DataPoolPtr = std::unique_ptr<DataPool>;

}
}
#endif
