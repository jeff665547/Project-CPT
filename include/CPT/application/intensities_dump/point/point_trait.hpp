#pragma once
#include <CPT/application/intensities_dump/point.hpp>
#include <opencv2/opencv.hpp>
namespace cpt { namespace application { namespace intensities_dump {
namespace point {

// struct LBB;

// template<class T>struct DomainCast{};
// 
// template<> struct DomainCast<LBB>
// {
//     template<class S, class E, class T>
//     static auto run(
//           const Point< LTB, S, E >& p
//         , const std::pair< T, T>& v 
//     )
//     {
//         return Point< LBB, S, E > {
//             p.x, v.first - p.y
//         };
//     }
// };
// template<> struct DomainCast<LTB>
// {
//     template<class S, class E, class T>
//     static auto run(
//           const Point< LBB, S, E >& p
//         , const std::pair< T, T>& v 
//     )
//     {
//         return Point< LTB, S, E > {
//             p.x, v.first - p.y
//         };
//     }
// };
// 
// template<class RD, class D, class S, class E, class T>
// auto domain_cast(
//       const Point< D, S, E >& p
//     , const std::pair< T, T>& v 
// )
// {
//     return DomainCast<RD>::run( p, v );
// }
// template<class S, class E>
// auto to_cv_point(
//     const Point< LTB, S, E >& p
// )
// {
//     return cv::Point( p.x, p.y );
// }

struct LBB
{
    // template<class S, class E, class T>
    // static auto add( 
    //       const Point< LTB, S, E >& p
    //     , const std::pair< T, T >& v 
    // )
    // {
    //     return Point< LTB, S, E >{ p.x + v.first, p.y - v.second };
    // }
    // template<class S, class E, class T>
    // static auto add( 
    //       const Point< LBB, S, E >& p
    //     , const std::pair< T, T >& v 
    // )
    // {
    //     return Point< LBB, S, E >{ p.x + v.first, p.y + v.second };
    // }
    // template<class S, class E, class T>
    // static auto sub( 
    //       const Point< LTB, S, E >& p
    //     , const std::pair< T, T >& v 
    // )
    // {
    //     return Point< LTB, S, E >{ p.x - v.first, p.y + v.second };
    // }
    // template<class S, class E, class T>
    // static auto sub( 
    //       const Point< LBB, S, E >& p
    //     , const std::pair< T, T >& v 
    // )
    // {
    //     return Point< LBB, S, E >{ p.x - v.first, p.y - v.second };
    // }
    // template<class S, class E, class T>
    // static auto sub( 
    //       const Point< LBB, S, E >& p0
    //     , const Point< LTB, S, E >& p1
    //     , const std::pair< T, T >& plant
    // )
    // {
    //     auto tmp = domain_cast<LBB>( p1, plant );
    //     return Point< LBB, S, E > {
    //         p0.x - p1.x, p0.y - tmp.y 
    //     };
    //     // struct QQQ{
    //     //     int x; int y;
    //     // };
    //     // return QQQ();
    // }
    // template<class S, class E, class T>
    // static auto sub( 
    //       const Point< LBB, S, E >& p0
    //     , const Point< LBB, S, E >& p1
    //     , const std::pair< T, T >& plant
    // )
    // {
    //     return Point< LBB, S, E > {
    //         p0.x - p1.x, p0.y - p1.y 
    //     };
    // }


    // template<class S, class E, class T>
    // static auto rect( 
    //       const Point< LTB, S, E >& p0
    //     , const Point< LTB, S, E >& p1 
    //     , const std::pair< T, T > & plant
    // )
    // {
    //     return cv::Rect(
    //           cv::Point( p0.x, p1.y )
    //         , cv::Point( p1.x, p0.y )
    //     );
    // }
    // template<class S, class E, class T>
    // static auto rect( 
    //       const Point< LBB, S, E >& p0
    //     , const Point< LBB, S, E >& p1 
    //     , const std::pair< T, T > & plant
    // )
    // {
    //     return cv::Rect(
    //           to_cv_point ( domain_cast< LTB > ( p0, plant ) )
    //         , to_cv_point ( domain_cast< LTB > ( p1, plant ) )
    //     );
    // }

};
struct LTB
{
    // template<class S, class E, class T>
    // static auto add( 
    //       const Point< LTB, S, E >& p
    //     , const std::pair< T, T >& v 
    // )
    // {
    //     return Point< LTB, S, E >{ p.x + v.first, p.y + v.second };
    // }
    // // template<class S, class E, class T>
    // // static auto add( 
    // //       const Point< LBB, S, E >& p
    // //     , const std::pair< T, T >& v 
    // // )
    // // {
    // //     return Point< LBB, S, E >{ p.x + v.first, p.y  v.second };
    // // }
    // template<class S, class E, class T>
    // static auto sub( 
    //       const Point< LTB, S, E >& p
    //     , const std::pair< T, T >& v 
    // )
    // {
    //     return Point< LTB, S, E >{ p.x - v.first, p.y - v.second };
    // }
    // template<class S, class E, class T>
    // static auto sub( 
    //       const Point< LBB, S, E >& p
    //     , const std::pair< T, T >& v 
    // )
    // {
    //     return Point< LBB, S, E >{ p.x - v.first, p.y - v.second };
    // }
};

}
}}}
