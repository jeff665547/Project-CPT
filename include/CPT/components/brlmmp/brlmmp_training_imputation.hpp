#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class BRLMMpTrainingImputation : public engine::NamedComponent
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

        monitor.set_monitor( "Component BRLMMp Training Imputation", 3 );
        monitor.log( "Component BRLMMp Training Imputation", "Imputate 1 Missing Cluster ... " );

        db.brlmmp.imputate_1_missing_cluster( db.brlmmp.buffers );

        monitor.log( "Component BRLMMp Training Imputation", "Imputate 2 Missing Cluster ..." );

        db.brlmmp.imputate_2_missing_cluster( db.brlmmp.buffers );

        monitor.log( "Component BRLMMp Training Imputation", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
