#pragma once

#ifdef NEW_DATA_POOL 
#include <CPT/components/rnd_choice2.hpp>
#else

#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
#include <vector>
#include <CPT/format/tsv.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <CPT/utility/allele_signals_loader.hpp>
#include <random>
#include <CPT/algorithm/probeset_rnd_choice.hpp>

namespace cpt {
namespace component {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ca = cpt::algorithm;
template<class DATA_POOL>
class RndChoice
{
    DATA_POOL data_pool_;
  public:
    using RndIds = typename ca::ProbesetSampleRndChoice::RndIds;
    std::size_t rnd_nums;
    ced::Shared<RndIds> rnd_probeset_ids;

    RndChoice( DATA_POOL&& data_pool, std::size_t default_rnd_nums )
    : data_pool_    ( std::forward<DATA_POOL>(data_pool) )
    , rnd_nums     ( default_rnd_nums                   )
    {}
    void config_parameters( const bpt::ptree& p ) 
    {
        /* TODO require cube */
        auto j ( cf::make_json( p ) );
        rnd_nums = j
            .get_optional< std::size_t >("rnd_nums")
            .value_or( rnd_nums );
    }
    template<class T>
    void require( ced::Shared<T>& s, const std::string& name )
    { // engine code
        if( name != "none" )
            data_pool_.require(s, name );
    }
    void require_list( const bpt::ptree& p )
    {
        auto json ( cf::make_json( p ) );
        require( rnd_probeset_ids, json.get<std::string>( "rnd_probeset_ids" ));
    }
    void initialize() 
    {
        data_pool_.hard_load(rnd_probeset_ids, rnd_nums);
    }
    void start() 
    {
        auto& rnd_choice_ids = rnd_probeset_ids.get();
        ca::ProbesetSampleRndChoice prc;
        rnd_choice_ids = prc.get_rnd_vec(
              data_pool_.cube
            , rnd_nums
        );
    }
    void finish() 
    {
        rnd_probeset_ids.release();
    }
    // ~RndChoice()
    // {
    //     cpt::dbg << "RndChoice destroy" << std::endl;
    // }
};

}}
#endif
