#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
namespace ced = cpt::engine::data_pool;
TEST ( data_paths_pool, exist_path_tag )
{
    ced::DataPathsPool dpp;
    dpp.push_path("path1", "/path1");
    dpp.push_path("path2", "/path2");
    EXPECT_EQ(dpp.get_path_list("path1").size(), 1);
    EXPECT_EQ(dpp.get_path_list("path2").size(), 1);
    EXPECT_EQ(dpp.exist_path_tag("path1"), true);
    EXPECT_EQ(dpp.exist_path_tag("path3"), false);
}
