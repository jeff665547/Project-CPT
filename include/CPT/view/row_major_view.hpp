#pragma once
#include <vector>
#include <cassert>
#include <Nucleona/language.hpp>
#include <Nucleona/language.hpp>
#include <CPT/improc/util.hpp>
namespace cpt{ namespace view{

template<class T>
struct RowMajorView
{
    using Width     = typename std::decay_t<T>::size_type;
    using Height    = typename std::decay_t<T>::size_type;
    using Value     = typename std::decay_t<T>::value_type&;
    
    RowMajorView(T&& o, Width width, Height height )
    : width_    ( width     )
    , height_   ( height    )
    , data_     ( FWD(o)    )
    {
        assert ( data_.size() == width_ * height_ );
    }
    GETTER( auto&, at( Height i, Width j ),
    {
        return data_.at( (i * width_) + j );
    })
    GETTER(auto&, operator()( Height i, Width j ), { return at( i, j ) ; })
    auto n_rows()                           { return height_    ; }
    auto n_cols()                           { return width_     ; }
    auto operator()( const cv::Rect& r )
    {
        std::decay_t<T> res;
        for ( auto i(r.y); i < r.y + r.height; i ++ )
        {
            for ( auto j(r.x); j < r.x + r.width; j ++ )
            {
                res.push_back( this->at(i,j) );
            }
        }
        return RowMajorView<std::decay_t<T>>( std::move(res), r.width, r.height );
    }
  protected:
    Width   width_      ;
    Height  height_     ;
  public:
    T       data_       ;
};
template<class T>
auto make_row_major_view ( T&& o, typename RowMajorView<T>::Width w, typename RowMajorView<T>::Height h)
{
    return RowMajorView<T>( FWD(o), w, h  );
}

}}
