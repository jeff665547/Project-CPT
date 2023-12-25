#pragma once
#include <mlpack/core.hpp>
#include <utility>
#include <armadillo>
#include <Nucleona/container/vector.hpp>
#include <CPT/utility/mutable.hpp>
#include <CPT/utility/typecheck.hpp>
#include <boost/range/irange.hpp>
#include <Nucleona/language.hpp>

namespace cpt {
namespace visualization {
namespace geometrics {
namespace cu = cpt::utility;
namespace md = mlpack::distribution;
namespace bt = boost;
namespace ellipse_detail {
// using namespace cu::typecheck_detail;
// template<class T> struct InputAdapter;
// template<class T> InputAdapter<T> iwrap( T&& o );
// 
// template<class T> 
// struct InputAdapter
// {
//     T storage;
// 
//     InputAdapter(T&& o )
//     : storage{ std::forward<T>(o) }
//     {}
//     template<class E>
//     static decltype(auto) get( arma::Col<E>& vec, std::size_t i )
//     {
//         return vec(i);
//     }
//     template<class E, class... R>
//     static decltype(auto) get( std::vector<E, R...>& vec, std::size_t i )
//     {
//         return vec.at(i);
//     }
//     template<class E>
//     static decltype(auto) get( arma::Mat<E>& mat, std::size_t i, std::size_t j )
//     {
//         return mat(i, j);
//     }
//     template<class E, class R>
//     static decltype(auto) get( nucleona::container::ndvector<2, E, R>& mat, std::size_t i, std::size_t j )
//     {
//         return mat.at(i).at(j);
//     }
//     template<class IN1, class IN2>
//     static decltype(auto) mul ( IN1&& in1, IN2&& in2 )
//     {
//         return in1 * in2;
//     }
//     template<class IN1, class IN2>
//     static decltype(auto) add ( IN1&& in1, IN2&& in2 )
//     {
//         return in1 + in2;
//     }
//     decltype(auto) operator()( std::size_t i )
//     {
//         return get(this->storage, i); 
//     }
//     decltype(auto) operator()( std::size_t i, std::size_t j )
//     {
//         return get(this->storage, i, j); 
//     }
//     template<class TT>
//     decltype(auto) operator* ( const InputAdapter<TT>& in )
//     {
//         return iwrap( mul(this->storage, in.storage));
//     }
//     template<class TT>
//     decltype(auto) operator+ ( const InputAdapter<TT>& in )
//     {
//         return iwrap ( add(this->storage, in.storage));
//     }
// };
// CREATE_TYPECHECKER(InputAdapter);
// template<class R>
// decltype(auto) base_trans ( R&& row )
// {
//     return trans( std::forward<R>(row) );
// }
// template< class V, FTP_TYPE_CHECK(V, InputAdapter) >
// decltype(auto) itrans( V&& o )
// {
//     return iwrap(base_trans( FWD(o.storage) ));
// }
// template<class T>
// InputAdapter<T> iwrap( T&& o )
// {
//     return InputAdapter<T>{std::forward<T>(o)};
// }
}
template<class EMAT, class SHIFT>
auto ellipse(EMAT&& emat, SHIFT&& shift)
{
    return [
          memat  = cu::mms(std::forward<EMAT >( emat  ))
        , mshift = cu::mms(std::forward<SHIFT>( shift ))
    ]( const double& theta ) mutable
    {
        arma::Row<double> rr { cos(theta), sin(theta) };
        return (
              arma::trans(rr * memat.storage) + mshift.storage
        ).eval();
    };
}
template<class GD_COV_MAT, class GD_MEAN
    , TP_TYPE_CHECK(GD_COV_MAT, arma::mat)
    , TP_TYPE_CHECK(GD_MEAN, arma::vec)
>
auto ellipse_gd_cov_mean( GD_COV_MAT&& gcm, GD_MEAN&& gm)
{
    arma::vec eigval;
    arma::mat eigvec;
    arma::eig_sym(eigval, eigvec, std::forward<GD_COV_MAT>(gcm));
    eigval = (arma::sqrt(eigval) * 3).eval();
    eigvec.each_col(
        [&eigval]( arma::vec& col )
        {
            col %= eigval;
        }
    );
    return ellipse( std::move(eigvec), std::forward<GD_MEAN>(gm) );
}
template<class T, TP_TYPE_CHECK(T, md::GaussianDistribution)>
auto ellipse_gd( T&& gd )
{
    return ellipse_gd_cov_mean ( gd.Covariance(), gd.Mean() );
}
auto test()
{
    md::GaussianDistribution gd;
    ellipse_gd(gd);
}
}}}
