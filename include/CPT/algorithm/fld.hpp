#pragma once
#include <mlpack/methods/gmm/gmm.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost_addon/range_filter.hpp>
#include <CPT/algorithm/accumulate.hpp>
#include <armadillo>
#include <CPT/algorithm/matrix_range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost_addon/range_glue.hpp>
namespace cpt {
namespace algorithm {
namespace mg = mlpack::gmm;
namespace bt = boost;
namespace ba = bt::adaptors;
std::tuple<double, double> fld( mg::GMM& model )
{
    auto value = 0.0;
    auto ratio = 1.0;
    auto&& n_comps = model.Gaussians();
    auto&& dim = model.Dimensionality();
    if ( n_comps > 1 )
    {
        std::vector<double> a;
        auto u = arma::diff(
            cu::irange_0(n_comps)
                | ba::transformed(
                    [&model](auto&& i )
                    {
                        return arma::trans(model.Component(i).Mean());
                    }
                )
                | row_rng_to_arma_mat( n_comps, dim )
        ).eval(); 
        for ( auto&& k : cu::irange_0(n_comps - 1 ))
        {
            auto&& S = ( bt::irange(k, k+2)
                | glue(ba::transformed(
                    [&model] ( auto&& k ) { 
                        return model.Component(k).Covariance();
                    }
                ))
                | accumulate( arma::mat(dim, dim, arma::fill::zeros) )
            )
            ;
            auto&& u_rk = arma::trans(u.row(k));
            auto&& w = arma::solve( S, u_rk );
            auto&& c = std::pow( arma::dot( w, u_rk ), 2 )
                / ( 
                    ( S * w )
                    | arma_dot ( w )
                )
            ; 
            a.emplace_back ( std::move(c) );
            value = *bt::min_element( a );
            ratio = *bt::max_element( a )/ *bt::min_element( a );
        }
    }
    return std_addon::make_tuple(
          std::move(value)
        , log2(ratio)
    );

}

}}
