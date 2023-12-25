#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/algorithm/argsort.hpp>
#include <CPT/algorithm/access.hpp>
#include <CPT/algorithm/print.hpp>
#include <boost_addon/range_eval.hpp>
namespace ca = cpt::algorithm;
TEST(argsort_test, range)
{
    std::vector<int> values {
        2, 4, 6, 7, 1, 3, 5
    };
    auto indexs ( values | ca::index_sort<std::vector<std::size_t>>() );
    values 
        | ca::idx_access( indexs ) 
        | ca::printed(std::cout, '\t') 
        | endp
    ;
    std::cout << std::endl;
}
