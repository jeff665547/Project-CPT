#pragma once
#include <CPT/range/core.hpp>
#include <boost_addon/range_vector.hpp>
namespace cpt {
namespace algorithm {
RANGE_NAMESPACE_SHORTCUT
struct Getlined
{
};
template<class __IN>
struct GetlinedRangeF : public cu::MutableStorage<__IN>
{
    using Base = cu::MutableStorage<__IN>;
    mutable std::string line;
    GetlinedRangeF(__IN&& in)
    : Base { std::forward<__IN>(in) }
    {}
    std::string operator()(bool& flag) const
    {
        flag = (bool)std::getline(this->storage, line);
        return line;
    }
};
template<class IS>
auto operator|(IS&& is, Getlined tag)
{
    return ::make_fwd_func_rng(
          GetlinedRangeF<IS>(std::forward<IS>(is))
        , true
        , false
    );
}
auto getlined()
{
    return Getlined();
};
/* example usage 
for(auto line : fin | getlined)
{
    std::cout << line << std::endl;
}
*/

}}
