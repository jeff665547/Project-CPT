#pragma once
#include <CPT/engine/online_components.hpp>
#include <CPT/utility/type_for_each.hpp>
/**
 * build rule into database or update if the rule is already exist
 * this module only run while webapp startup
 */
namespace cpt
{
namespace engine
{
template<class COMP>
struct BuildRule
{
    static void run()
    {
        /* TODO build rule thing */
        COMP::build_rule();
    }
};
struct RuleBuilder
{
    void operator()()
    {
        TypeForEach<OnlineComponents, BuildRule>::run();
    }
};
}
}
