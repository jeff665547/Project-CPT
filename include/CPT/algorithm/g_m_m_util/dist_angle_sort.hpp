#pragma once
/* sort by angle */
#include <mlpack/methods/gmm/gmm.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <type_traits>
#include <Nucleona/type_traits/core.hpp>
#include <CPT/algorithm/state_flattened.hpp>
#include <CPT/algorithm/access.hpp>
#include <boost_addon/range_glue.hpp>
#include <CPT/utility/mutable.hpp>
#include <CPT/algorithm/argsort.hpp>
#include <CPT/algorithm/mdl_components.hpp>
namespace cpt {
namespace algorithm {
namespace g_m_m_util {
namespace mg = mlpack::gmm;
namespace md = mlpack::distribution;
namespace ba = boost::adaptors;
namespace bt = boost;
namespace cu = cpt::utility;
namespace ca = cpt::algorithm;

template<class VEC>
auto permute_dist_by_arg( const mg::GMM& m, VEC& vec)
{
    std::vector<md::GaussianDistribution> newdist;
    arma::vec weight(vec.size());
    for ( auto&& e : vec | glue(ba::indexed(0)) )
    {
        weight[e.index()] = m.Weights()[e.value()];
        newdist.emplace_back(m.Component(e.value()));
    }
    return mg::GMM(newdist, weight);
}
template<class MDL>
auto dist_angle_sort(MDL&& model)
{
    using SizeRow = arma::Row<std::size_t>;
    cpt::algorithm::ArgSort argsort;
    auto ordered_args ( 
        argsort.subj_valuef<SizeRow>(
            model | ca::mdl_components()
            , []( auto&& subj, auto&& sid )
            {
                auto&& mean = subj[sid].Mean();
                std::complex<double> dot(mean[0], mean[1]);
                return std::arg(dot);
            }
    ) );
    // rebuild gmm
    return permute_dist_by_arg( model, ordered_args ) ;
    // model = ( permute_dist_by_arg( model, ordered_args ) );
}


}}}
