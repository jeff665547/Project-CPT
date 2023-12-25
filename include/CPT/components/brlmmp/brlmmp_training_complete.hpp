#pragma once
#include <CPT/engine/components/named_component.hpp>
#include <CPT/engine/data_pool/component_object_manager/icp_obj.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace component {

class BRLMMpTrainingComplete : public engine::NamedComponent
{
    using Base = engine::NamedComponent;

    std::string clustering_models_;

  protected:

    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        auto& root = db.pipeline_schema();

        for( auto& child : root.get_child( "context.clustering_models" ))
        {
            clustering_models_ = child.second.get_value< std::string >();
        }
    }

  public:

    using Base::Base;

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();

        monitor.set_monitor( "Component BRLMMp Training Complete", 1 );
        db.brlmmp.output_training( db.brlmmp.buffers, clustering_models_ );
        monitor.log( "Component BRLMMp Training Complete", "Complete!!!" );
    }
};

} // end of namespace component
} // end of namespace cpt
