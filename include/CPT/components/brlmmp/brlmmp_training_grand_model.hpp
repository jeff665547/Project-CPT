#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class BRLMMpTrainingGrandModel : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

  public:

    using Base::Base;

    virtual void initialize() override
    {
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component BRLMMp Training Grand Model", 3 );
        monitor.log( "Component BRLMMp Training Grand Model", "Building ... " );
        for( size_t i = 0; i < db.probeset_likelihood_clusters.size(); ++i )
        {
            db.probeset_likelihood_clusters[i][ db.probeset_bic_clusters[i][0].second ].second.delete_data();
            db.brlmmp.buffers.emplace_back( std::move( db.probeset_likelihood_clusters[i][ db.probeset_bic_clusters[i][0].second ].second ));
            db.brlmmp.bic_buffer_idxs_emplace( db.probeset_bic_clusters[i][0].second, db.brlmmp.buffers.size() );
        }

        db.probeset_likelihood_clusters.clear();
        db.probeset_bic_clusters.clear();

        monitor.log( "Component BRLMMp Training Grand Model", "Building ... " );

        db.brlmmp.build_model( db.brlmmp.buffers );

        monitor.log( "Component BRLMMp Training Grand Model", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
