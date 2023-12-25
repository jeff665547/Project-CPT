#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/engine/data_pool/component_object_manager.hpp>
#include <CCD/utility/language.hpp>
#include <string>
#include <fstream>
using namespace std::string_literals;

namespace com_ = cpt::engine::data_pool::component_object_manager;
TEST(icp_obj, constructer)
{
    const std::string raw_sym_tab_path(nucleona::language::sdir(__FILE__)  + "/icp_obj_test/icp_obj_test.json"s );
    auto raw_sym_tab = cpt::format::read_json( raw_sym_tab_path );
    com_::CPSymTab::read_json_config( raw_sym_tab.get_child("context").root );
    std::cout << "read table finish" << std::endl;
    com_::CPSymTab::show_info( std::cout );
    EXPECT_TRUE( 
           com_::CPSymTab::get_entry("testsym")->get<std::string>() 
        == raw_sym_tab.get<std::string>("context.testsym")
    );
    auto obj(
        com_::require<std::ifstream>(
            raw_sym_tab.get_child("test_obj")
        )
    );
    std::cout << "an object created" << std::endl;
    com_::CPSymTab::show_info( std::cout );
    obj->show_info(std::cout);

    com_::CPSymTab::initialize();
    std::cout << "symtab initialized" << std::endl;
    com_::CPSymTab::show_info( std::cout );
    obj->show_info(std::cout);
    EXPECT_TRUE(
        com_::CPSymTab::get_data().empty()
    );

    obj->initialize();
    std::cout << "object initialized" << std::endl;
    com_::CPSymTab::show_info( std::cout );
    obj->show_info(std::cout);
    auto& fin ( obj->get() );
    std::string line;
    while ( std::getline(fin, line))
    {
        std::cout << line << std::endl;
    }

}
TEST(icp_obj, constructer_no_param)
{
    using NS = std::tuple<int, std::string>;
    const std::string raw_sym_tab_path(nucleona::language::sdir(__FILE__)  + "/icp_obj_test/icp_obj_test.json"s );
    auto raw_sym_tab = cpt::format::read_json( raw_sym_tab_path );
    auto obj(
        com_::require<std::ifstream>(
            raw_sym_tab.get_child("test_obj")
        )
    );
    std::cout << "an object created" << std::endl;
    com_::CPSymTab::show_info( std::cout );
    obj->show_info(std::cout);

    com_::CPSymTab::initialize();
    std::cout << "symtab initialized" << std::endl;
    com_::CPSymTab::show_info( std::cout );
    obj->show_info(std::cout);
    EXPECT_TRUE(
        com_::CPSymTab::get_data().empty()
    );

    obj->initialize();
    std::cout << "object initialized" << std::endl;
    com_::CPSymTab::show_info( std::cout );
    obj->show_info(std::cout);
    auto& fin ( obj->get() );
    fin.open(raw_sym_tab.get<std::string>("context.testsym"));
    std::string line;
    while ( std::getline(fin, line))
    {
        std::cout << line << std::endl;
    }

}
TEST(icp_obj, no_ref)
{
    using NS = std::tuple<int, std::string>;
    const auto raw_sym_tab_path( nucleona::language::sdir(__FILE__) + "/icp_obj_test/icp_obj_test2.json"s );

    auto raw_sym_tab = cpt::format::read_json( raw_sym_tab_path );
    com_::CPSymTab::read_json_config( raw_sym_tab.get_child("context").root );
    com_::CPSymTab::initialize();
    std::cout << "symtab initialized" << std::endl;
    com_::CPSymTab::show_info( std::cout );
}
