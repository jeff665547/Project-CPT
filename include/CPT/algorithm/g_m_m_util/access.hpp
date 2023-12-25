#pragma once
#include <mlpack/methods/gmm/gmm.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <type_traits>
#include <Nucleona/type_traits/core.hpp>
#include <CPT/algorithm/state_flattened.hpp>
#include <CPT/algorithm/access.hpp>
#include <boost_addon/range_glue.hpp>
#include <CPT/utility/mutable.hpp>
namespace cpt {
namespace algorithm {
namespace g_m_m_util {
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace ba = boost::adaptors;
namespace bt = boost;
namespace cu = cpt::utility;
// template<class GMM>
// struct GMMAccessor : public cu::MutableStorage<GMM>
// {
//     template<class I>
//     decltype(auto) operator()(I&& i)
//     {
//         return this->storage.Component(i);
//     }
// };
template<class T, class CH = nucleona::type_traits::TypeCheckP<T, mg::GMM> >
auto components( T&& gmm )
{
    return 
        bt::irange(std::size_t(0), gmm.Gaussians()) 
            | glue(ba::transformed(
                [g = cu::mms(std::forward<T>(gmm))]( auto&& i ) 
                    -> decltype(auto)
                {
                    return g.storage.Component(i);
                }
            ))
    ;
}
class GMMMeanTag
{
  public:
    template<class T>
    auto operator()( T&& gmm ) const
    {
        return components(std::forward<T>(gmm))
            | glue(ba::transformed(
                []( auto&& comp ) 
                    -> decltype(auto)
                {
                    return comp.Mean();
                }
            ))
        ;
    }
};
template<class RNG>
auto operator|( RNG&& rng, const GMMMeanTag& t)
{
    return t(std::forward<RNG>(rng));
}
auto meaned()
{
    return GMMMeanTag();
}
struct GMMCovarianceTag
{
    template<class T>
    auto operator()( T&& gmm ) const
    {
        return components(std::forward<T>(gmm))
            | glue(ba::transformed(
                []( auto&& comp ) 
                    -> decltype(auto)
                {
                    return comp.Covariance();
                }
            ))
        ;
    }
};
template<class RNG>
auto operator|( RNG&& rng, const GMMCovarianceTag& gmmct)
{
    return gmmct(std::forward<RNG>(rng));
}
auto covarianced()
{
    return GMMCovarianceTag();
}
// template<class T, class CH = std_addon::TypeCheckP<T, mg::GMM> >
// auto flatten_covariances(T&& gmm)
// {
//     return components( std::forward<T>(gmm) )
//         | ba::transformed(
//             []( auto&& comp ) -> auto&
//             {
//                 return comp.Covariance();
//             }
//         )
//         | state_flatten()
//     ;
// }
// template<class T, class CH = std_addon::TypeCheckP<T, mg::GMM> >
// auto flatten_means(T&& gmm)
// {
//     return components( std::forward<T>(gmm) )
//         | ba::transformed(
//             []( auto&& comp ) -> auto&
//             {
//                 return comp.Mean();
//             }
//         )
//         | state_flattened()
//     ;
// }

}}}


