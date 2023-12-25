#pragma once
#include <boost/type_traits.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator.hpp>
#include <boost/optional.hpp>
#include <functional>
#include <boost/range/iterator_range.hpp>
#include <Nucleona/language.hpp>
#include <CPT/utility/mutable.hpp>
#include <boost_addon/function_range/core.hpp>
#include <CPT/range/core.hpp>
#include <CPT/utility/irange.hpp>
namespace cpt {
namespace range {
namespace segment {
RANGE_NAMESPACE_SHORTCUT
template<class RNG> struct SegmentIter;

template<class RNG>
using SegmentIterBase = boost::iterator_facade<
      SegmentIter<RNG>
    , typename boost::iterator_range<typename RNG::iterator> /* value */
    , typename boost::forward_traversal_tag
    , typename boost::iterator_range<typename RNG::iterator>
    , typename boost::iterator_difference<typename RNG::iterator>
>;
template<class RNG>
struct SegmentIter
: public SegmentIterBase<RNG>
{
    using Base                  = SegmentIterBase<RNG>           ;
    using Reference             = typename Base::reference       ;
    using Value                 = typename Base::value_type      ;
    using Difference            = typename Base::difference_type ;
    using This                  = SegmentIter<RNG>               ;
    using ChildIter             = typename RNG::iterator         ;
    using Base::Base;

    RNG&            main_range  ;
    std::size_t     segsize     ;
    ChildIter       beg         ;
    
    SegmentIter( RNG& rng, std::size_t _segsize)
    : main_range    ( rng )
    , segsize       ( _segsize )
    , beg           ( rng.begin() )
    {}
    SegmentIter( RNG& rng, std::size_t _segsize, ChildIter&& _beg )
    : main_range    ( rng       )
    , segsize       ( _segsize  )
    , beg           ( _beg      )
    {}

  private: 
    static auto end_iter( const ChildIter& rbeg, const ChildIter& tend, std::size_t ss )
    {
        auto tmp (rbeg + ss);
        return tmp >= tend ? tend : tmp;
    }
    static auto beg_iter( const ChildIter& tbeg, const ChildIter& rbeg, std::size_t ss )
    {
        auto tmp (rbeg - ss);
        return tmp < tbeg ? tbeg : tmp;
    }
    static auto end_iter( const ChildIter& rbeg, const ChildIter& tend )
    {
        return rbeg >= tend ? tend : rbeg;
    }
    static auto iter_norm( const ChildIter& rbeg, const RNG& rng ) 
    {
        if( rbeg < rng.begin() ) return rng.begin();
        if( rbeg >= rng.end() ) return rng.end();
        return rbeg;
    }
    static auto is_in ( const ChildIter& itr, const RNG& rng )
    {
        return itr < rng.end() && itr >= rng.begin();
    }
    friend class boost::iterator_core_access;
    void increment() 
    {
        beg += segsize;
    }
    bool equal ( const This& o ) const
    {
        // special for end
        if ( !is_in ( beg, main_range ) )
            if ( !is_in( o.beg, o.main_range ) )
                return true;
        return beg == o.beg && segsize == o.segsize;
    }
    Reference dereference() const
    {
        return boost::make_iterator_range(
              iter_norm( beg, main_range )
            , iter_norm( beg + segsize, main_range )
        );
    }
    void decrement()
    {
        beg -= segsize;
    }
    void advance( const Difference& o )
    {
        beg += (o * segsize);
    }
    Difference distance_to( const This& o ) const 
    {
        return (o.value.begin() - beg)/segsize;
    }
};
template<class RNG>
auto make_segment_iter ( RNG& rng, std::size_t segsize )
{
    return SegmentIter<RNG>(rng, std::move(segsize));
}
template<class RNG>
auto make_segment_end_iter ( RNG& rng, std::size_t segsize )
{
    return SegmentIter<RNG>(
          rng
        , std::move(segsize)
        , rng.end()
    );
}
}}}
