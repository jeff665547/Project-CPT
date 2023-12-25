#pragma once
#include <utility>
#include <CPT/utility/typecheck.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost_addon/range_glue.hpp>
#include <CPT/const_tags.hpp>
#include <CPT/utility/mutable.hpp>
#include <Nucleona/language.hpp>
#include <CPT/algorithm/lazy_foreach.hpp>
namespace cpt {
namespace algorithm {
namespace ba = boost::adaptors;
namespace cu = cpt::utility;
template<class Tag>
struct PrintdImpl
{};
template<> struct PrintdImpl<ImmDo>
{
    template<class OS, class DEL, class T>
    auto operator()(OS&& os, DEL& del, T&& o) const
    {
        auto itr = o.begin();
        os << *itr; itr++;
        for(;itr != o.end(); itr++)
        {
            os << del << *itr;
        }
        return std::forward<T>(o);
    }
};
template<> struct PrintdImpl<LateDo>
{
    template<class OS, class DEL, class T>
    auto operator()(OS&& os, DEL& del, T&& o)
    {
    }
};

template<class TRAIT, class OS, class DEL>
struct Printed : public PrintdImpl<TRAIT>
{
    OS os;
    DEL del; 
    Printed(OS&& _os, DEL&& _del)
    : os(std::forward<OS>(_os))
    , del( std::forward<DEL>(_del))
    {}
    template<class T>
    auto operator()(T&& o)
    {
        return PrintdImpl<TRAIT>::operator()(
              std::forward<OS>(os)
            // , std::forward<DEL>(del)
            , del
            , std::forward<T>(o)
        );
    }
};
CREATE_TYPECHECKER(Printed);
template<class OS, class DEL>
auto printed(OS&& os, DEL&& del)
{
    return cpt::algorithm::lazy_foreach(
        cu::mutable_([
              first = true
            , del_ = cu::mms(std::forward<DEL>(del))
            , oss = cu::mms(std::forward<OS>(os))
        ]( auto&& v ) mutable -> DERET_V(v)
        {
            if( first ) 
            {
                oss.storage << v;
                first = false;
            }
            else oss.storage << del_.storage << v;
            return FWD(v);
        })
    );   
}
template<class OS, class DEL, class OF>
auto printed(OS&& os, DEL&& del, OF&& of)
{
    return cpt::algorithm::lazy_foreach(
        cu::mutable_([
              first = true
            , del_ = cu::mms(std::forward<DEL>(del))
            , oss = cu::mms(std::forward<OS>(os))
            , off = of
        ]( auto&& v ) mutable -> DERET_V(v)
        {
            if( first ) 
            {
                off(oss.storage, v);
                first = false;
            }
            else off( oss.storage << del_.storage, v );
            return FWD(v);
        })
    );   
}
template<class OS, class DEL>
auto iprinted(OS&& os, DEL&& del)
{
    return Printed<ImmDo, OS, DEL>(
        std::forward<OS>(os)
        , std::forward<DEL>(del)
    );
}

template<class RNG, class ROP, FTP_TYPE_CHECK(ROP, Printed)>
auto operator|(RNG&& rng, ROP&& rop)
{
    return std::forward<ROP>(rop)(std::forward<RNG>(rng));
}
}}
