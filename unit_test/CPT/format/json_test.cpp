#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/format/json.hpp>
#include <boost/property_tree/ptree.hpp>
TEST(json, basic_test)
{
    namespace bpt = boost::property_tree;
    auto json = cpt::format::make_json();
    json.add("a", 0);
    json.add("c.c.d", 111);
    auto json2 = cpt::format::make_json();
    json2.add("ab", 1);
    json2.add("ab", 2);
    json.add("b", json2);
    std::vector<std::string> list{"asdf", "wert", "ffff"};
    json.add("list.content", list);
    auto jlist = json.create_list("nums");
    jlist.push_back(0);
    jlist.push_back(1);
    jlist.push_back(2);
    auto jlist2 = cpt::format::make_json_list();
    jlist2.push_back(7);
    jlist2.push_back(3);
    jlist.push_back(jlist2);
    auto json3 = cpt::format::make_json();
    json3.add("child", 456);
    jlist.push_back(json3);
    auto qqq = json.create_child("qqq");
    qqq.add("df.as", "dfhdgh");
    json.dump(std::cout);
}
// TEST(json, basic_test2)
// {
// bad case of first level is list
//     namespace bpt = boost::property_tree;
//     auto jlist = cpt::format::make_json_list();
//     jlist.push_back(0);
//     jlist.push_back(1);
//     jlist.dump(std::cout);
// }
