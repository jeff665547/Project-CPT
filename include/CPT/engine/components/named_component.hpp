#pragma once 
#include <CPT/engine/component.hpp>
#include <CPT/engine/data_pool.hpp>
#include <Nucleona/language.hpp>
namespace cpt
{
namespace engine
{
namespace components
{

class NamedComponentImpl : public cpt::engine::Component
{
protected :
    virtual void config_parameters ( const bpt::ptree& parameters )
    {}
public :
    using Base = cpt::engine::Component;
    NamedComponentImpl( 
          const DataPool& data_pool
        , const bpt::ptree& schema_node
    )
    :Base ( data_pool, schema_node )
    {
    }
    virtual void config ( const bpt::ptree& node_schema ) override
    {
        auto&& parameters = node_schema
            .get_child("parameter");
        this->config_parameters(parameters);
    }
};
CREATE_DERIVED_TYPE( NamedComponent, NamedComponentImpl );

}
}
}
