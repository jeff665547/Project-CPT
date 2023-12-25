#pragma once
#include <mlpack/methods/gmm/gmm.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <type_traits>
#include <Nucleona/type_traits/core.hpp>
#include <CPT/algorithm/g_m_m_util/access.hpp>
namespace cpt {
namespace algorithm {
namespace mg = mlpack::gmm;
namespace ba = boost::adaptors;
namespace bt = boost;
struct MdlComponents {};
constexpr MdlComponents mdl_components ()
{
    return MdlComponents();
}
// http://stackoverflow.com/questions/20709896/how-do-i-use-stdenable-if-with-a-self-deducing-return-type
// template<
//       class T
//     , class DISABLE = std_addon::disable_if_t<
//         ::decay_equiv_v<T, MdlComponents>, int
//     >
// >
template<class T>
auto operator| ( T&& gmm, MdlComponents c)
{
    return g_m_m_util::components( std::forward<T>(gmm) );
}

}}
