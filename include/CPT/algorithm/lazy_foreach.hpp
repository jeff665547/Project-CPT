#pragma once
#include <utility>
#include <CPT/utility/typecheck.hpp>
#include <Nucleona/language.hpp>
#include <boost/iterator_adaptors.hpp>
namespace cpt{ namespace algorithm{

template<class FUNC>
struct LazyForeach
{
    LazyForeach( FUNC&& func )
    : func_ ( std::forward<FUNC>( func ) )
    {}

    template<class... ARGS>
    decltype(auto) operator()(ARGS&&... args) const 
    {
        return func_ ( FWD(args)... );
    }

    template<class... ARGS>
    decltype(auto) operator()(ARGS&&... args) 
    {
        return func_ ( FWD(args)... );
    }
private:
    FUNC func_;
};
template< class FUNC, class ITER > struct LazyForeachIter; // forward declartion
template< class FUNC, class ITER >
using LazyForeachIterProto = boost::iterator_adaptor<
      LazyForeachIter<FUNC, ITER>
    , ITER
    , typename std::decay< 
        std::result_of_t< 
            FUNC( typename std::iterator_traits<ITER>::reference  ) 
        > 
    >::type
    , typename std::iterator_traits<ITER>::iterator_category
    , typename std::result_of< 
        FUNC( typename std::iterator_traits<ITER>::reference ) 
    >::type
    , typename std::iterator_traits<ITER>::difference_type
>;

template< class FUNC, class ITER >
struct LazyForeachIter : public LazyForeachIterProto< FUNC, ITER >
{
private:
    friend class boost::iterator_core_access;
    using Base = LazyForeachIterProto< FUNC, ITER >;
    decltype(auto) dereference() const
    {
        return (*func_) ( *this->base_reference() );
    }
    FUNC* const func_ { nullptr };
public:
    template<class... ARGS>
    LazyForeachIter( FUNC& func, ARGS&&... args )
    : Base  ( FWD(args)... )
    , func_ ( &func )
    {}

    LazyForeachIter()
    : Base()
    , func_ ( nullptr )
    {}
    // LazyForeachIter( const LazyForeachIter& iter )
    // : Base  ( (const Base&)iter )
    // , func_ ( iter.func_ )
    // {}

    // LazyForeachIter( LazyForeachIter&& iter )
    // : Base  ( std::move((Base&)iter) )
    // , func_ ( iter.func_ )
    // {}
};

template<class RNG, class FUNC>
struct LazyForeachRange
{
private: 
    RNG rng_;
    FUNC func_;

public:
    using iterator          = LazyForeachIter< 
          std::remove_reference_t<FUNC>
        , std::conditional_t< 
              std::is_const< std::remove_reference_t< RNG > >::value
            , typename std::remove_reference_t<RNG>::const_iterator
            , typename std::remove_reference_t<RNG>::iterator
        >
    >;
    using reference         = typename iterator::reference  ;
    using value_type        = typename iterator::value_type ;
    using const_iterator    = LazyForeachIter< 
          std::conditional_t< 
              std::is_reference<FUNC>::value
            , std::remove_reference_t<FUNC>
            , const typename std::decay_t<FUNC>
        >
        , std::conditional_t< 
              std::is_reference<RNG>::value
              , std::conditional_t< 
                    std::is_const< std::remove_reference_t<RNG> >::value
                  , typename std::remove_reference_t<RNG>::const_iterator
                  , typename std::remove_reference_t<RNG>::iterator
              >
              , typename std::decay_t<RNG>::const_iterator
        >
    >;

    LazyForeachRange( RNG&& rng, FUNC&& func)
    : rng_ ( FWD(rng) )
    , func_ ( FWD( func ) )
    {} 
    auto begin()        { return iterator( func_, rng_.begin() );       }
    auto end()          { return iterator( func_, rng_.end()   );       }
    auto begin() const  { return const_iterator( func_, rng_.begin() );     }
    auto end() const    { return const_iterator( func_, rng_.end() );       }

    template<class I>
    decltype(auto) operator[] ( const I& i )
    {
        return func_( rng_[i] );
    }



};

template<class FUNC>
decltype(auto) lazy_foreach( FUNC&& func )
{
    return LazyForeach<FUNC>( std::forward<FUNC>(func) );
}
CREATE_TYPECHECKER(LazyForeach);

template<class RNG, class LF, FTP_TYPE_CHECK(LF, LazyForeach) >
decltype(auto) operator| ( RNG&& rng, LF&& lf )
{
    return LazyForeachRange<RNG, LF>( std::forward<RNG>(rng), FWD(lf) );
}

}}
