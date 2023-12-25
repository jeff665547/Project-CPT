#pragma once
#include <algorithm>
#include <boost/range/irange.hpp>
#include <armadillo>
#include <CPT/utility/type.hpp>
namespace cpt {
namespace algorithm {
namespace cu = cpt::utility;
template<class T>
struct ArgTypeBase
{

};
template<class T>
struct ArgTypeBase<std::vector<T>>
{
    using Type = std::size_t;
};
template<class T>
using ArgType = typename ArgTypeBase<T>::Type;
template<class T>
struct ArgGen
{
};
template<class T>
struct ArgGen<arma::Row<T>>
{
    template<class SUBJ>
    static auto run( SUBJ& subj )
    {
        auto&& size = boost::distance(subj);
        using Size = std::decay_t<decltype(size)>;
        arma::Row<T> res(size);
        for( auto&& i : boost::irange(Size(0), size) )
        {
            res[i] = i;
        }
        return res;
    }
};
template<class T>
struct ArgGen<std::vector<T>>
{
    template<class SUBJ>
    static auto run( SUBJ& subj )
    {
        auto dis(boost::distance(subj));
        auto&& ir = boost::irange(decltype(dis)(0), boost::distance(subj));
        return std::vector<T>( ir.begin(), ir.end() );
    }
};
struct ArgSort
{
    template<class T>
    auto& value( const std::vector<T>& vec, std::size_t i )
    {
        return vec[i];
    }

    template<class ARG = std::vector<std::size_t>, class SUBJ, class VALUEF>
    ARG subj_valuef( const SUBJ& subj, VALUEF&& valuef)
    {
        auto arg( ArgGen<ARG>::run( subj ));
        std::sort( arg.begin(), arg.end()
            , [&valuef, &subj]( const auto& i1, const auto& i2 )
            {
                return valuef(subj, i1) < valuef(subj, i2);
            }
        );
        return arg;
    }
    template<class SUBJ, class ARG, class COMP>
    auto operator()( const SUBJ& subj, ARG&& arg, COMP&& comp )
    {
        std::sort(arg.begin(), arg.end(), comp);
    }
    template<class SUBJ, class ARG>
    auto operator()( const SUBJ& subj, ARG&& arg )
    {
        this->operator()(subj, std::forward<ARG>(arg)
            , [this, &subj]( const auto& v1, const auto& v2  )
            {
                return value(subj, v1) < value(subj, v2);
            }
        );
    }
    template<class SUBJ>
    auto operator()( const SUBJ& subj)
    {
        auto ir ( boost::irange( 0, subj.size() ) );
        std::vector<ArgType<SUBJ>> arg(ir.begin(), ir.end());
        std::sort(arg.begin(), arg.end()
            , [this, &subj]( const auto& v1, const auto& v2  )
            {
                return value(subj, v1) < value(subj, v2);
            }
        );
    }
};
struct IndexSortTag{};

template<class RNG, class ArgT>
auto operator|(RNG&& rng, std::tuple<IndexSortTag, cu::Type<ArgT>> as )
{
    auto&& arg( ArgGen<ArgT>::run( rng ));
    std::sort( arg.begin(), arg.end()
        , [&rng]( const auto& i1, const auto& i2 )
        {
            return rng[i1] < rng[i2];
        }
    );
    return std::forward<ArgT>(arg);

}
template<class RNG, class ArgT, class COMP>
auto operator|(RNG&& rng, std::tuple<IndexSortTag, cu::Type<ArgT>, COMP> as )
{
    auto&& arg( ArgGen<ArgT>::run( rng ) );
    auto&& comp ( std::get<2>(as ));
    std::sort( arg.begin(), arg.end()
        , [&rng, &comp]( const auto& i1, const auto& i2 )
        {
            return comp( rng[i1], rng[i2] );
        }
    );
    return std::forward<ArgT>(arg);
}
template<class ARG_T>
auto index_sort()
{
    return std::tuple<IndexSortTag, cu::Type<ARG_T>>();
}
template<class ARG_T, class COMP>
auto index_sort(COMP&& comp)
{
    return std::tuple<IndexSortTag, cu::Type<ARG_T>, COMP>(
        {}, {}, std::forward<COMP>(comp)
    );
}

}}
