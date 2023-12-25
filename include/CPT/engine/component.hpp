#pragma once
#include <CPT/engine/data_pool.hpp>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <CPT/forward.hpp>
/**
 * the framework of component
 * do not add any logical code in this section
 */
namespace cpt
{
namespace engine
{
class ComponentImpl
{
  protected: 
    const DataPool& data_pool_;
  public:
    using DataPoolType = DataPool;
    ComponentImpl ( 
          const DataPool& data_pool
        , const bpt::ptree& schema_node
    )
    : data_pool_ ( data_pool )
    {
    }
    static  void build_rule () {}
    virtual void initialize() {}
    virtual void config ( const bpt::ptree& parameter ) {}

    virtual void start      () {}
    virtual void finish     () {}
    virtual void operator() ()
    {
        this->start();
        this->finish();
    }
    DataPool& mut_data_pool()
    {
        return const_cast<DataPool&>(data_pool_);
    }
    // std::size_t num_samples ( void ) const
    // {
    //     return data_pool_.get_path_list("cel_path").size();
    // 
    // }
    // std::size_t num_probes(void) const
    // {
    //     return 
    //         static_cast<size_t>(
    //             this->data_pool_
    //                 .cdf
    //                 .num_rows
    //         ) 
    //         * this->data_pool_.cdf.num_cols;
    // }
    virtual ~ComponentImpl() {}
};
using Component = ComponentImpl;
using ComponentPtr = std::unique_ptr<Component>;
using NamedComponent = components::NamedComponent;
}
}
