
#pragma once
#include <vector>
#include <cassert>
#include <Nucleona/language.hpp>
#include <string>
#include <CPT/view/detail.hpp>
#include <Nucleona/language.hpp>
namespace cpt{ namespace view{

template<class T>
struct MatrixTransformView
{
    using FuncTrait = cpt::view::MatrixTrait<T>;
    using I     = typename FuncTrait::I;
    using J     = typename FuncTrait::J;
    using Value = typename FuncTrait::Value;
    using TansFuncType = std::function< Value ( I, J ) >;

    MatrixTransformView(
          T&& o
        , const std::string& op
        , const char& x_dir
    )
    : mat_( FWD( o ) )
    {
        if      ( op == cpt::view::MatViewConst::LT && x_dir == '_')
        {
            trans_ = [ mat = FWD(o) ]( I i, J j ) mutable -> auto& { return mat( i, j ); };
        }
        else if ( op == cpt::view::MatViewConst::LT && x_dir == '|')
        {
            // cv::transpose( m, m );
            trans_ = [ mat = FWD(o) ]( I i, J j )  mutable -> auto& { return mat( j, i ); };
        }
        else if ( op == cpt::view::MatViewConst::LB && x_dir == '_' )
        { // flip upside down
            // cv::flip ( m, m, 0 );
            trans_ = [ mat = FWD(o) ]( I i, J j ) mutable -> auto&  
            { 
                return mat( mat.n_rows() - 1 - i, j ); 
            };
        }
        else if ( op == cpt::view::MatViewConst::LB && x_dir == '|' )
        { // rotate 90 degree
            // transpose(m, m);  
            // flip( m, m, 1 );
            trans_ = [ mat = FWD(o) ]( I i, J j ) mutable -> auto&
            { 
                return mat( j, mat.n_cols() - 1 - i );
            };
        }
        else
        {
            throw std::logic_error( "unknown coordinate system" );
        }
    }
    T_GETTER( template<class... IDX>, auto&, at( IDX&&... idx ), { return trans_( FWD(idx)... ); } )
    T_GETTER( template<class... IDX>, auto&,  operator()( IDX&&... idx ),  { return at( FWD(idx)... ); } )
  protected:
    T mat_ ;

    TansFuncType trans_;
};
template<class T>
auto make_matrix_transform_view ( 
      T&& o
    , const std::string& op
    , const char& x_dir
)
{
    return MatrixTransformView<T>( FWD(o), op, x_dir );
}


}}
