#include <CPT/algorithm/matrix_range/submatrix.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <armadillo>
#include <CPT/algorithm/matrix_range.hpp>
#include <CPT/algorithm/access.hpp>
#include <boost_addon/range_eval.hpp>
#include <CPT/algorithm/print.hpp>
#include <CPT/utility/mutable.hpp>

namespace ca = cpt::algorithm;
namespace cam = cpt::algorithm::matrix_range;
namespace ba = boost::adaptors;
namespace cu = cpt::utility;
template<class T>
void print_type()
{
    std::cout << __PRETTY_FUNCTION__ <<std::endl;
}
TEST(mat, col_range)
{
    std::stringstream ss;
    arma::Mat<int> mat {
        {  0,  1 },
        { 20, 30 },
        { 11, 21 }
    };
    mat 
    | ca::col_range() 
    | ca::lazy_foreach(
        [&ss]( auto&& v )
        {
            v.for_each(
                [&ss]( auto&& vv )
                {
                    ss << vv << ',';
                }
            );
            return v;
        }
    )
    | endp;
    EXPECT_EQ( ss.str(), "0,20,11,1,30,21," );
}
TEST(mat, row_range)
{
    std::stringstream ss;
    arma::Mat<int> mat {
        {  0,  1 },
        { 20, 30 },
        { 11, 21 }
    };
    mat 
    | ca::row_range() 
    | ca::lazy_foreach(
        [&ss]( auto&& v )
        {
            v.for_each(
                [&ss]( auto&& vv )
                {
                    ss << vv << ',';
                }
            );
            return v;
        }
    )
    | endp;
    EXPECT_EQ( ss.str(), "0,1,20,30,11,21," );
}
TEST(mat, mixed)
{
    arma::Mat<int> mat {
        {  0,  1,  2,  3,  4,  5 },
        { 20, 30, 40, 50, 60, 70 }, // 1
        { 11, 21, 33, 54, 91, 32 },
        { 12, 44, 57, 66, 33, 67 }  // 3
    };
    auto submat = mat 
    | ca::row_range()
    | ca::idx_access({ 1, 3 })
    | ca::lazy_foreach(
        [](auto row) // subview object
        {
            return std::move(row) | ca::idx_access({2, 4});
        }
    )
    ;
    EXPECT_EQ(submat[0][0], 40);
    EXPECT_EQ(submat[1][1], 33);

    submat[0][0] = 0;
    EXPECT_EQ( mat(1,2), 0 );

    submat[1][1] = 0;
    EXPECT_EQ( mat(3,4), 0 );
}
TEST(mat, submatrix)
{
    std::stringstream ss;
    arma::Mat<int> mat {
        {  0,  1,  2,  3,  4,  5 },
        { 20, 30, 40, 50, 60, 70 },
        { 11, 21, 33, 54, 91, 32 }
    };
    auto&& submat = (mat 
        | cam::submatd(
              std::vector<int>{0, 1}
            , std::vector<int>{2, 4, 5, 3}
        )).tview()
/*
 * 2  4  5  3
 * 40 60 70 50
 */
        | ca::lazy_foreach(
            [&ss](auto&& col)
            {
                col | ca::printed( ss, ',' ) | endp;
                ss << ' ';
                return col;
            }
        )
        | ::endp
    ;
    EXPECT_EQ( ss.str(), "2,40 4,60 5,70 3,50 " );
}
