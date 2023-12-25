#pragma once
#include <boost_addon/range_indexed.hpp>
#include <armadillo>
#include <boost/range/adaptors.hpp>
#include <boost/range/irange.hpp>
#include <Nucleona/type_traits/core.hpp>
#include <CPT/algorithm/matrix_range.hpp>
#include <CPT/algorithm/access.hpp>
#include <CPT/algorithm/matrix_range/trait.hpp>
#include <CPT/utility/mutable.hpp>
#include <CPT/algorithm/lazy_foreach.hpp>
namespace cpt {
namespace algorithm {
namespace matrix_range {
namespace ba = boost::adaptors;
namespace bt = boost;
namespace cu = cpt::utility;
struct RowidxTag{};
template<class IDXS>
auto row_idxd(IDXS&& idxs)
{
    return std_addon::make_tuple(
        RowidxTag(), make_vector(std::forward<IDXS>(idxs))
    );
}
struct ColidxTag{};
template<class IDXS>
auto col_idxd(IDXS&& idxs)
{
    return std_addon::make_tuple(
        ColidxTag(), make_vector(std::forward<IDXS>(idxs))
    );
}
template<class RNG, class IDXS>
auto row_idx( RNG&& rng, IDXS&& idxs )
{
    return std::forward<RNG>(rng)
        | row_range()
        | idx_access( std::forward<IDXS>(idxs) )
    ;
}
template<class RNG, class IDXS>
auto col_idx( RNG&& rng, IDXS&& idxs )
{
    return std::forward<RNG>(rng)
        | col_range()
        | idx_access( std::forward<IDXS>(idxs) )
    ;
}
template<class RNG, class IDXS>
auto operator|(RNG&& rng, std::tuple<RowidxTag, IDXS> tup)
{
    return row_idx(
          std::forward<RNG>(rng)
        , std::forward<RURDT<IDXS>>(std::get<1>(tup))
    );
}
template<class RNG, class IDXS>
auto operator|(RNG&& rng, std::tuple<ColidxTag, IDXS> tup)
{
    return col_idx(
          std::forward<RNG>(rng)
        , std::forward<RURDT<IDXS>>(std::get<1>(tup))
    );
}
template<class ROW_RNG, class COLT>
struct ColView
{
    ROW_RNG row_rng;
    COLT col_base;
    auto range() const
    {
        return row_rng
            | cpt::algorithm::lazy_foreach(
                [this](auto&& i)
                {
                    return col_base[i];
                }
            )
        ;
    }
    
};
template<class ROW_RNG, class COLT>
auto make_col_view(ROW_RNG&& row_rng, COLT&& col_base)
{
    return ColView<ROW_RNG, COLT>{std::forward<ROW_RNG>(row_rng), std::forward<COLT>(col_base)};
}
template<class MAT, class RIS, class CIS>
struct SubmatView
{
    std::size_t  n_rows;
    std::size_t  n_cols;
    MAT          mat;
    RIS          ris;
    CIS          cis;
    using Mat = MAT;
    decltype(auto) view()
    {
        return ris
            | cpt::algorithm::lazy_foreach(
                [this](auto i)
                {
                    return cis
                        | cpt::algorithm::lazy_foreach(
                            cu::mutable_(
                            [ row = mat.row(i)](auto j) mutable -> auto&
                            {
                                return row[j];
                            })
                        )
                    ;
                }
            )
        ;
    }
    decltype(auto) tview()
    {
        return cis
            | cpt::algorithm::lazy_foreach(
                [this](auto i)
                {
                    return ris
                        | cpt::algorithm::lazy_foreach(
                            cu::mutable_(
                            [col = mat.col(i)](auto j) mutable -> auto&
                            {
                                return col[j];
                            })
                        )
                    ;
                }
            )
        ;
    }
};
template<class Mat, class RIS, class CIS>
auto make_submat_view(
      std::size_t       n_rows
    , std::size_t       n_cols
    , Mat           &&  mat
    , RIS           &&  ris
    , CIS           &&  cis
)
{
    return SubmatView<Mat, RIS, CIS>{ 
          std::move(n_rows)
        , std::move(n_cols)
        , std::forward<Mat>(mat)
        , make_vector(ris)
        , make_vector(cis)
    };
}
struct SubmatTag
{
    template<
          class T
        , class RIDXS
        , class CIDXS
        , class CH = std::enable_if_t<
            is_arma_mat<std::decay_t<T>>
        >
    >
    auto operator()(T&& o, RIDXS&& ridxs, CIDXS&& cidxs)
    {
        return make_submat_view(
              boost::distance(ridxs)
            , boost::distance(cidxs)
            , std::forward<T>(o)
            , std::forward<RIDXS>(ridxs)
            , std::forward<CIDXS>(cidxs)
        )
        ;
    }
};
struct SubmatViewToArmaMatTag
{
    // template<class Mat, class RIS, class CIS>
    // auto operator()(SubmatView<Mat, RIS, CIS>& sv)
    template<class SV>
    auto operator()(SV&& sv)
    {
        using MatType = std::decay_t<typename SV::Mat> ;
        MatType m(sv.n_rows, sv.n_cols);
        for( auto&& c : sv.tview() | ::range_indexed(0))
        {
            m.col(c.index()) = c.value() | to_arma_vec();
        }
        return m;
    }
};

auto submat_to_arma_mat()
{
    return SubmatViewToArmaMatTag();
}
template<class RNG>
auto operator|( RNG&& rng, SubmatViewToArmaMatTag tag)
{
    return tag(std::forward<RNG>(rng));
}
template<
      class RIDXS
    , class CIDXS
>
auto submatd(RIDXS&& ridxs, CIDXS&& cidxs)
{
    return std_addon::make_tuple(
          SubmatTag()
        , std::forward<RIDXS>(ridxs)
        , std::forward<CIDXS>(cidxs)
    );
}
template<
      class T
    , class RIDXS
    , class CIDXS
    , class CH = std::enable_if_t<
        is_arma_mat<std::decay_t<T>>
    >
>
auto operator|(T&& o, std::tuple<SubmatTag, RIDXS, CIDXS> tup)
{
    return std::get<0>(tup)(
          std::forward< T   >(o)
        , std::forward<::RURDT<RIDXS>>(std::get<1>(tup))
        , std::forward<::RURDT<CIDXS>>(std::get<2>(tup))
    );
}

}}}
