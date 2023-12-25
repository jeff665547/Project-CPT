#pragma once
#include <boost_addon/range_indexed.hpp>
#include <armadillo>
#include <boost/range/adaptors.hpp>
#include <boost/range/irange.hpp>
#include <Nucleona/type_traits/core.hpp>
#include <CPT/algorithm/matrix_range/trait.hpp>
#include <CPT/utility/irange.hpp>
#include <boost_addon/range_glue.hpp>
#include <CPT/utility/typecheck.hpp>
namespace cpt {
namespace algorithm {
namespace ba = boost::adaptors;
namespace bt = boost;
namespace cu = cpt::utility;

namespace matrix_range_detail {
struct MatrixInfo
{
    const std::size_t n_rows;
    const std::size_t n_cols;
    MatrixInfo ( std::size_t n_rows, std::size_t n_cols )
    : n_rows(n_rows)
    , n_cols(n_cols)
    {}
};
template<
      class T
    , class I
    , std::enable_if_t<
          matrix_range::is_arma_mat<std::decay_t<T>> 
        , int
    > = 0 
>
decltype(auto) get_col( T&& m, I i )
{
    return m.col(i);
}
// template<class T, class I>
template<
      class T
    , class I
    , std::enable_if_t<
          matrix_range::is_arma_row<std::decay_t<T>> 
        , int
    > = 0 
>
decltype(auto) get_col( T&& r, I i )
{
    return r[i];
}
// template<class T, class I>
template<
      class T
    , class I
    , std::enable_if_t<
          matrix_range::is_arma_mat<std::decay_t<T>> 
        , int
    > = 0 
>
decltype(auto) get_row( T&& m, I i )
{
    return m.row(i);
}
// template<class T, class I>
template<
      class T
    , class I
    , std::enable_if_t<
          matrix_range::is_arma_col<std::decay_t<T>> 
        , int
    > = 0 
>
decltype(auto) get_row( T&& c, I i )
{
    return c[i];
}
}
struct ColRngToArmaMatrix 
: public matrix_range_detail::MatrixInfo
{
    using Base = matrix_range_detail::MatrixInfo;
    using Base::Base;
    
    template<class M, class RNG>
    void operator()( M&& m, RNG&& rng )
    {
        for( auto&& col : rng | range_indexed(0) )
        {
            m.col(col.index()) = col.value();
        }
    }
};
auto col_rng_to_arma_mat( const std::size_t& n_rows, const std::size_t& n_cols )
{
    return ColRngToArmaMatrix { n_rows, n_cols };
}
template<class RNG>
auto operator|( RNG&& rng, ColRngToArmaMatrix tam )
{
    using col_type  = typename std::decay_t<RNG>::value_type;
    using elem_type = typename col_type::elem_type;
    arma::Mat<elem_type> m ( tam.n_rows, tam.n_cols );
    tam(m, rng);
    return m;
}


struct RowRngToArmaMatrix
: public matrix_range_detail::MatrixInfo
{
    using Base = matrix_range_detail::MatrixInfo;
    using Base::Base;
    template<class M, class RNG>
    void operator()( M&& m, RNG&& rng )
    {
        for( auto&& row : rng | range_indexed(0) )
        {
            m.row(row.index()) = row.value();
        }
    }
};
auto row_rng_to_arma_mat( 
      const std::size_t& n_rows
    , const std::size_t& n_cols 
)
{
    return RowRngToArmaMatrix { n_rows, n_cols };
}
template<class RNG>
auto operator|( RNG&& rng, RowRngToArmaMatrix tam )
{
    using row_type  = typename std::decay_t<RNG>::value_type;
    using elem_type = typename row_type::elem_type;
    arma::Mat<elem_type> m ( tam.n_rows, tam.n_cols );
    tam(std::move(m), std::forward<RNG>(rng));
    return m;
}



struct ColRangeTag
{
    template<class M>
    auto operator()( M&& m )
    {
        using elem_type  = typename std::decay_t<M>::value_type;
        return bt::irange(m.n_cols - m.n_cols, m.n_cols)
            | glue(ba::transformed(
                [mat = cu::mms(std::forward<M>(m))]( auto i ) -> decltype(auto)
                {
                    return matrix_range_detail::get_col(mat.storage, i);
                }
            ))
        ;
    }
};
auto col_range() { return ColRangeTag(); }
template<class T>
auto operator|( T&& mat, ColRangeTag crt )
{
    return crt( std::forward<T>(mat) );
}




struct RowRangeTag
{
    template<class M>
    auto operator()( M&& m ) const
    {
        using elem_type  = typename std::decay_t<M>::value_type;
        return bt::irange(m.n_rows - m.n_rows, m.n_rows)
            | glue(ba::transformed(
                [mat = cu::mms(std::forward<M>(m))]( auto i ) -> decltype(auto)
                {
                    return matrix_range_detail::get_row(mat.storage, i);
                }
            ))
        ;
    }
};
auto row_range() { return RowRangeTag(); }
template<class T>
auto operator|( T&& mat, RowRangeTag crt )
{
    return crt(std::forward<T>(mat));
}

struct SliceRangeTag
{
    template<class C>
    auto operator()( C&& c ) const
    {
        using elem_type  = typename std::decay_t<C>::value_type;
        return cu::irange_0(c.n_slices)
            | glue(ba::transformed(
                [cube = cu::mms(std::forward<C>(c))]( auto i ) -> decltype(auto)
                {
                    return cube.storage.slice(i);
                }
            ))
        ;
    }
};
auto slice_range() { return SliceRangeTag(); }
template<class T>
auto operator|( T&& mat, SliceRangeTag crt )
{
    return crt(std::forward<T>(mat));
}

template< template < class... E > class T >
struct ToArmaVec
{   
    // template<class... R>
    // using RT = T<R...>;
};
auto to_arma_vec()
{
    return ToArmaVec<arma::Col>();
}
auto to_arma_row()
{
    return ToArmaVec<arma::Row>();
}
template<class RNG, template< class...E > class T >
auto operator|( RNG&& rng, const ToArmaVec<T>& tav )
{
    using Value = typename std::decay_t<RNG>::value_type;
    
    T<Value> res( boost::distance( rng ) );
    for( auto&& v : rng | ::range_indexed(0))
    {
        res[v.index()] = v.value();
    }
    return res;
}
template<class T, template< class...E > class T2 >
auto operator|( const std::vector<T>& rng, const ToArmaVec<T2>& tav )
{
    T2<T> res( rng );
    return res;
}
template<class P>
struct ArmaDot
{
    P p;
};
CREATE_TYPECHECKER(ArmaDot);
template<class P>
auto arma_dot(P&& p)
{
    return ArmaDot<P>{ std::forward<P>(p) };
}
template<class RNG, class AD, FTP_TYPE_CHECK(AD, ArmaDot)>
auto operator|(RNG&& rng, AD&& ad )
{
    return arma::dot(
          std::forward<RNG>(rng)
        , FWD(ad.p)
    );
}

struct ArmaTrans
{};
// CREATE_TYPECHECKER(ArmaTrans);
auto arma_trans()
{
    return ArmaTrans{};
}
template<class RNG>
auto operator|(RNG&& rng, const ArmaTrans& at )
{
    return arma::trans(std::forward<RNG>(rng));
}

}}
