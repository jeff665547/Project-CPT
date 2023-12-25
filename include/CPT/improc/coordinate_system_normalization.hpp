#pragma once
#include <opencv2/opencv.hpp>
#include <CPT/view/detail.hpp>
namespace cpt { namespace improc {
// enum OriginPos
// {
//       LT = 0
//     , LB = 1
//     , RT = 2
//     , RB = 3
// };
enum XAxisDirect
{
    H, V
};
template<class MAT, class... T>
struct VMat : public MAT
{};
struct CoordinateSystemNormalization
{
    CoordinateSystemNormalization() = default;

    // constexpr static const char* const LT = "LT";
    // constexpr static const char* const LB = "LB";
    

    template<class T>
    auto operator () ( 
          cv::Mat_<T>& m
        , const std::string& op
        , const char& x_dir 
    )
    {
        if      ( op == cpt::view::MatViewConst::LT && x_dir == '_')
        {}
        else if ( op == cpt::view::MatViewConst::LT && x_dir == '|')
        {
            cv::transpose( m, m );
        }
        else if ( op == cpt::view::MatViewConst::LB && x_dir == '_' )
        { // flip upside down
            cv::flip ( m, m, 0 );
        }
        else if ( op == cpt::view::MatViewConst::LB && x_dir == '|' )
        { // rotate 90 degree
            transpose(m, m);  
            flip( m, m, 1 );
        }
        else
        {
            throw std::logic_error( "unknown coordinate system" );
        }
    }
    template<class T>
    auto operator()(
          std::vector<T>& v
        , std::size_t col_nums
        , const std::string& op
        , const char& x_dir 
    )
    {
        cv::Mat_<int> tmp(cv::Size(0, 0), CV_32SC1);;
        cv::Mat_<int> mask;
        for ( typename std::vector<T>::size_type i = 0; i < v.size(); i ++ )
        {
            tmp.push_back( (int)i );
        }
        mask = tmp.reshape( 1, (v.size() / col_nums) );

        this->operator()( mask, op, x_dir );
        {
            std::vector<T> res;
            for ( int i = 0; i < mask.rows; i ++ )
            {
                for ( int j = 0 ; j < mask.cols; j ++ )
                {
                    res.push_back( v.at( mask( i, j ) ) );
                }
            }
            v.swap(res);
        }

    }
};

}
}
