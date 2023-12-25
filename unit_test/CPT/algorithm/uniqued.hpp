#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/algorithm/argsort.hpp>
#include <CPT/algorithm/uniqued.hpp>
#include <boost/range/adaptor/transformed.hpp>
namespace ba = boost::adaptors;
namespace ca = cpt::algorithm;
TEST( unique, event_test)
{
    std::vector<int> vec { 1,3,5,7,2,4,6,3,6,3,2,2,1 };
    auto&& idx ( vec | ca::index_sort<std::vector<int>>() );
    ca::unique(
          idx.begin()
        , idx.end()
        , [&vec](auto&& r, auto&& f)
        {
            return vec[r] == vec[f];
        }
        , []( auto&& r, auto&& f)
        {
            f = r + 1;
        }
        , []( auto&& r, auto&& f)
        {
            f = r;
        }
    );
    for ( auto&& v : idx | ba::transformed(
            [&vec](auto&& i)
            {
                return vec[i];
            }
        )
    )
    {
        std::cout << v << ' ';
    }
    std::cout << std::endl;
}

