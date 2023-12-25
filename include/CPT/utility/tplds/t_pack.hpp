#pragma once

namespace cpt {
namespace utility {
namespace tplds {

template<class T, T... ts>
struct TPack
{
    using ElemType = T;
};

}}}
