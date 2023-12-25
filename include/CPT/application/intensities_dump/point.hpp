#pragma once
// #include <CPT/application/intensities_dump/point/point_trait.hpp>
#include <opencv2/opencv.hpp>
namespace cpt { namespace application {
namespace intensities_dump {


// template< class DOMAIN_, class SIZE_T = std::size_t, class EXT = void >
// class Point
// {
//   public:
//     using size_type = SIZE_T;
//     using Domain = DOMAIN_;
//     SIZE_T x;
//     SIZE_T y;
// 
// };
// template<class RD> struct DomainCast{};
// template<> struct DomainCast<point::LBB>
// {
//     template<class E, class S, class T>
//     static auto run( 
//         const Point<point::LTB, S, E>& p, const std::pair<T,T>& plant 
//     )
//     {
// 
// 
//     }
// };
// 
// template<class RDOMAIN, class E, class S, class T, class D>
// auto domain_cast( const Point<D, S, E>& p, const std::pair<T,T>& plant )
// {
// 
// }
// template<class E, class S, class T0, class T1, class D>
// auto operator+( const Point<D, S, E>& p, const std::pair<T0, T1>& v )
// {
//     return Point<D, S, E>{ p.x + v.first, p.y + v.second };
// }
// template<class E, class S, class T, class D>
// auto operator-( const Point<D, S, E>& p, const std::pair<T, T>& v )
// {
//     return Point<D, S, E>{ p.x - v.first, p.y - v.second };
// }
// namespace point {
//     struct LBB;
//     struct LTB;
// }
// template<class S, class E>
// auto make_cv_rect( 
//       const Point<point::LBB, S, E >& p0
//     , const Point<point::LBB, S, E >& p1 
// )
// {
//     return cv::Rect(
//           cv::Point( p0.x, p1.y )
//         , cv::Point( p1.x, p0.y )
//     );
// }
// template<class S, class E>
// auto make_cv_rect( 
//       const Point<point::LTB, S, E >& p0
//     , const Point<point::LTB, S, E >& p1 
// )
// {
//     return cv::Rect(
//           cv::Point( p0.x, p0.y )
//         , cv::Point( p1.x, p1.y )
//     );
// }

}
}}
