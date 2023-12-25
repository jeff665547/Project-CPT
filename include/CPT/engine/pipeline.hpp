#pragma once
#include <vector>
#include <CPT/engine/component.hpp>
#include <CPT/engine/data_pool/component_object_manager/cpsym_tab.hpp>
#include <CPT/logger.hpp>
namespace cpt
{
namespace engine
{

template<class LIST>
class PipelineImpl : public LIST
{
protected:
    using Base = LIST;
public :
    PipelineImpl ( LIST&& component_list )
    : Base ( std::move( component_list ) )
    {}

    void operator() ()
    {
        for( auto& component_ptr : (*this) )
        {
            component_ptr->initialize();
            (*component_ptr)();
            component_ptr.reset();
            // data_pool::component_object_manager::CPSymTab::show_info(cpt::dbg);
        }
    }
};
template<class LIST>
using Pipeline = PipelineImpl<LIST>;

template<class LIST>
auto make_pipeline( LIST&& component_list )
{
    static_assert ( std::is_rvalue_reference<LIST&&>::value, "");
    return Pipeline<std::decay_t<LIST>> ( 
        std::move( component_list ) 
    );
}

}
}
