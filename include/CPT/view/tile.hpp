#pragma once
#include <vector>
#include <cassert>
#include <Nucleona/language.hpp>
#include <Nucleona/language.hpp>
#include <CPT/view/row_major_view.hpp>
#include <CPT/improc/util.hpp>

namespace cpt{ namespace view{

template<class T, class TILES>
struct Tile
{
    Tile( T&& o, TILES&& tiles, std::size_t cols, std::size_t rows )
    : data_     ( FWD(o)        )
    , tiles_    ( FWD(tiles), cols, rows )
    {
        // assert(  
        //        data_.rows == tiles_.n_rows() 
        //     && data_.cols == tiles_.n_cols()
        // );
    }
    GETTER( auto&, tile( int i, int j ),
    {
        return tiles_(i, j);
    })
    GETTER( auto, at( int i, int j ),
    {
        return data_(tiles_(i, j));
    })
    GETTER(auto, operator()( int i, int j ), 
    { 
        return at( i, j ); 
    })
    auto n_rows() { return tiles_.n_rows()    ; }
    auto n_cols() { return tiles_.n_cols()    ; }
    auto operator()( const cv::Rect& r )
    {
        auto st = tiles_(r);
        auto cols = st.n_cols();
        auto rows = st.n_rows();
        return Tile<T&, std::decay_t<TILES>>(
              data_
            , std::move(st.data_)
            , cols
            , rows
        );
    }
    auto imwrite( const std::string& name )
    {
        auto tmp = data_.clone();
        for ( auto&& t : tiles_.data_ )
        {
            cv::rectangle( 
                  tmp
                , t 
                , cv::Scalar(30000)
            );
        }
        cpt::improc::imwrite( name, tmp );
    }
    auto imwrite( const std::string& name, std::size_t i, std::size_t j )
    {
        auto tmp = data_.clone();
        auto t = tiles_(i,j);
        cv::rectangle( 
              tmp
            , t 
            , cv::Scalar(30000)
        );
        cpt::improc::imwrite( name, tmp );
    }
    T data_;
  protected:
    RowMajorView<TILES> tiles_;
};
auto parse_tile_mat_rc( const std::vector<cv::Rect>& tiles )
{
    std::size_t x = 0;
    std::size_t cols = 0;
    for ( auto&& t : tiles )
    {
        if ( x > t.x )
        {
            break;
        }
        else
        {
            x = t.x;
            cols ++;
        }
    }
    assert( tiles.size() % cols == 0);
    return std::make_pair( tiles.size() / cols, cols );
}
template<class T, class TILES>
auto make_tile( T&& o, TILES&& tiles)
{
    auto rc = parse_tile_mat_rc( tiles );
    return Tile<T, TILES>( FWD(o), FWD(tiles), rc.second, rc.first  );
}


}}
