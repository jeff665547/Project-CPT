#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
#include <vector>
#include <CPT/format/tsv.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <CPT/utility/allele_signals_loader.hpp>
#include <random>
#include <CPT/algorithm/probeset_rnd_choice.hpp>
#include <CPT/engine/data_pool/component_object_manager.hpp>

namespace cpt {
namespace component {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ca = cpt::algorithm;
namespace com_ = cpt::engine::data_pool::component_object_manager;
template<class DATA_POOL>
class RndChoice
{
  public:
    using RndIds = typename ca::ProbesetSampleRndChoice::RndIds;
  private:
    DATA_POOL data_pool_;
    /* input */
    com_::ICPObjPtr<cpt::format::Cube<double>> probeset_cube_;

    /* output */
    com_::ICPObjPtr<RndIds> rnd_probeset_ids_;
    
  public:
    std::size_t rnd_nums;

    RndChoice( DATA_POOL&& data_pool, std::size_t default_rnd_nums )
    : data_pool_    ( std::forward<DATA_POOL>(data_pool) )
    , rnd_nums     ( default_rnd_nums                   )
    {}
    void config_parameters( const bpt::ptree& p ) 
    {
        auto j ( cf::make_json( p ) );

        rnd_nums = j
            .get_optional< std::size_t >("rnd_nums.content")
            .value_or( rnd_nums );

        rnd_probeset_ids_ = com_::require_w<RndIds>(
            j, "rnd_probeset_ids", com_::make_ref_parameter("GrandModelProbesetIds")
        );
        probeset_cube_ = com_::require_w< cpt::format::Cube<double> >( 
              j, "probeset_cube"
            , com_::make_ref_parameter("probeset_cube")
        );


    }
    void initialize() 
    {
        rnd_probeset_ids_   -> initialize();
        probeset_cube_      -> initialize();
    }
    void start() 
    {
        auto& rnd_probeset_ids = rnd_probeset_ids_  ->  get();
        auto& probeset_cube    = probeset_cube_     ->  get();

        ca::ProbesetSampleRndChoice prc;
        rnd_probeset_ids = prc.get_rnd_vec(
              probeset_cube
            , rnd_nums
        );
    }
    void finish() 
    {
        rnd_probeset_ids_   ->  release();
        probeset_cube_      ->  release();
    }
};

}}
#else
#endif
