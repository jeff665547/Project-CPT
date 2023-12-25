#pragma once

#include <CPT/engine/components/named_component.hpp>
#include <CPT/logger.hpp>
#include <vector>
#include <CPT/format/tsv.hpp>
#include <CPT/format/static_tsv_reader.hpp>
#include <CPT/utility/allele_signals_loader.hpp>
// #include <CPT/engine/data_pool/shared_data.hpp>
#include <CPT/engine/data_pool/shared_data_manager.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <CPT/algorithm/allele_min.hpp>
#include <CPT/algorithm/allele_max.hpp>
#include <CPT/algorithm/allele_avg.hpp>
#include <boost_addon/range_eval.hpp>
#include <boost_addon/range_sink.hpp>
#include <CPT/components/rnd_choice.hpp>
#include <CPT/components/birdseed/typedef.hpp>

namespace cpt {
namespace component {
namespace birdseed {
namespace cf = cpt::format;
namespace cu = cpt::utility;
namespace ced = cpt::engine::data_pool;
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
namespace grand_model_probeset_choice_detail {
    using GrandModelProbesetChoiceBase = engine::NamedComponent;
}

class GrandModelProbesetChoice 
: public grand_model_probeset_choice_detail::GrandModelProbesetChoiceBase
{
    using Base = grand_model_probeset_choice_detail::GrandModelProbesetChoiceBase;
    using DataPoolType = typename Base::DataPoolType;
  public:
    RndChoice<DataPoolType&>         rnd_choice       ;
    using RndIds = typename RndChoice<DataPoolType&>::RndIds;

    template< class... T >
    GrandModelProbesetChoice(
          const DataPoolType& data_pool
        , const T&... o
    )
    : Base          ( data_pool             , o... )
    , rnd_choice    ( this->mut_data_pool() , 5000 )
    {}
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        /* require cube */
        auto& db( this->mut_data_pool() );

        /* simulate data route */
        auto json ( cf::make_json() );
        // json.add( "rnd_probeset_ids", "GrandModelProbesetIds" );
        rnd_choice.config_parameters( p );
        // rnd_choice.require_list ( json.root );
    }
    virtual void initialize() override
    {
        /* load cube */
        rnd_choice.initialize();
    }
    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component BirdSeed Grand Model PS Select", 2 );
        monitor.log( "Component BirdSeed Grand Model PS Select", "Start" );

        rnd_choice.start();

        monitor.log( "Component BirdSeed Grand Model PS Select", "End" );
    }
    virtual void finish() override
    {
        rnd_choice.finish();
    }
};

}}}
