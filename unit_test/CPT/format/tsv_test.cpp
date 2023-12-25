#include <CPT/format/tsv.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <CCD/utility/tune_scope.hpp>
#include <CCD/utility/memory_check.hpp>
#include <fstream>

TEST( tsv, get )
{
    cpt::format::Tsv<> tsv;
    /* register needed column */
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("BMI");

    /* push data entry */
    tsv.push_entry("john", 74, 30.3);
    tsv.push_entry("quality", 11, 22.9);
    tsv.push_entry("qqqq", 20, 17.2);

    assert ( tsv.get<std::string>( 0, "name") == "john" ); /* the parameter of get is "coordinate" */
    assert ( tsv.get<double>( 2, "BMI") == 17.2 );

    tsv.get<std::string>( 2, "name" ) = "jack"; /* get's result is reference type */
    std::cout << tsv.get<std::string>( 2, "name" ) << std::endl;
    assert ( tsv.get<std::string>( 2, "name") == "jack" );
}
TEST( tsv, test )
{
    cpt::format::Tsv<> tsv;
    /* register needed column */
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("BMI");

    /* push data entry */
    tsv.push_entry("john", 74, 30.3);
    tsv.push_entry("quality", 11, 22.9);
    tsv.push_entry("qqqq", 20, 17.2);
    
    /* SQL like dataset query */
    tsv.ref().select("name").where([](const auto& o) /* where is "linear search" */
        {
            return (o.template get<int>("id") > 12);
        }
    ).view().dump(std::cout); /* dump to any std::ostream derive type */

    std::cout << "=======================\n";
    
    tsv.ref().select("name", "BMI").where([](const auto& o)
        {
            return (o.template get<std::string>("name").length() < 5);
        }
    ).view().dump(std::cout, false /* show no header */ ); 
}
TEST( tsv, test2 )
{
    cpt::format::Tsv<> tsv;
    /* register needed column */
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("BMI");

    /* push data entry */
    auto john = tsv.create_entry();
    john.get_variant("name") = "john";
    john.get_variant("id") = 74;
    john.get_variant("BMI") = 30.3;

    auto quality = tsv.create_entry();
    quality.get_variant("name") = "quality";
    quality.get_variant("id") = 11;
    quality.get_variant("BMI") = 22.9;

    auto qqqq = tsv.create_entry();
    qqqq.get_variant("name") = "qqqq";
    qqqq.get_variant("id") = 20;
    qqqq.get_variant("BMI") = 17.2;

    
    /* SQL like dataset query */
    {
        std::vector<std::string> labels ({ "name" });
        tsv.ref().select(labels).where([](const auto& o) /* where is "linear search" */
            {
                return (o.template get<int>("id") > 12);
            }
        ).view().dump(std::cout); /* dump to any std::ostream derive type */
    }

    std::cout << "=======================\n";

    { 
        std::vector<std::string> labels ({ "name", "BMI" });
        tsv.ref().select(labels).where([](const auto& o)
            {
                return (o.template get<std::string>("name").length() < 5);
            }
        ).view().dump(std::cout, false /* show no header */ ); 
    }
}
TEST( tsv, apply )
{
    cpt::format::Tsv<> tsv;
    /* register needed column */
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("BMI");

    /* push data entry */
    tsv.push_entry("john", 74, 30.3);
    tsv.push_entry("quality", 11, 22.9);
    tsv.push_entry("qqqq", 20, 17.2);
    
    /* SQL like dataset query */
    tsv.ref().select("name", "BMI").where([](const auto& o) /* where is "linear search" */
        {
            return (o.template get<int>("id") > 12);
        }
    ).modify().apply([](auto& o)
        {
            o.get_variant("name") = o.template get<std::string>("name") + "_test";
        } 
    /* apply modify function to table*/
    ).view().dump(std::cout); 

    tsv.ref().select("name", "BMI").where([](const auto& o) /* where is "linear search" */
        {
            return (o.template get<int>("id") > 12);
        }
    ).view().dump(std::cout); /* check modified content is apply to original data */
}
TEST( tsv, create_entry_if_not_exist )
{
    cpt::format::Tsv<> tsv;
    /* register needed column */
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("BMI");

    auto entry1 ( tsv.create_entry_if_not_exist("name", std::string("john")) );
    entry1.get_variant("id") = 12;
    entry1.get_variant("BMI") = 29.1;

    auto entry2 ( tsv.create_entry_if_not_exist("name", std::string("john")) );
    assert ( entry2.get<int>("id") == 12 );
    assert ( entry2.get<double>("BMI") == 29.1 );

    tsv.ref().select("name", "BMI", "id").where( [](const auto& o){ return true; } ).view().dump(std::cout);
}
TEST( tsv, exist)
{
    cpt::format::Tsv<> tsv;
    /* register needed column */
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("BMI");

    auto entry1 ( tsv.create_entry_if_not_exist("name", std::string("john")) );
    entry1.get_variant("id") = 12;
    entry1.get_variant("BMI") = 29.1;

    assert( tsv.exist("name", std::string("john")) == true );
    tsv.ref().select("name", "BMI", "id").where( [](const auto& o){ return true; } ).view().dump(std::cout);
}
struct Person
{
    int id;
    std::string name;
    float weight;
    std::vector<float> measurements;
};
TEST( tsv, create_frame )
{
    using namespace cpt::format;
    cpt::format::Tsv<> tsv;
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("weight");
    Person person { 0, "john", 76.3 };

    auto frame = tsv.create_frame();
    FRAME_SET_PROPERTY( frame, person, id );
    FRAME_SET_PROPERTY( frame, person, name );
    FRAME_SET_PROPERTY( frame, person, weight );
    frame.flush();
    tsv.ref()
        .select("id", "name", "weight")
        .where([](const auto& o){return true;})
        .view().dump(std::cout);
}
TEST( tsv, create_frame2 )
{
    using namespace cpt::format;
    cpt::format::Tsv<> tsv;
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("weight");
    tsv.register_column("measurements");
    Person person { 0, "Jane", 76.3, {30.1, 30.2, 30.3} };

    auto frame = tsv.create_frame();
    FRAME_SET_PROPERTY( frame, person, id );
    FRAME_SET_PROPERTY( frame, person, name );
    FRAME_SET_PROPERTY( frame, person, weight );
    FRAME_SET_PROPERTY( frame, person, measurements );
    frame.flush();
    tsv.ref()
        .select("id", "name", "weight", "measurements")
        .where([](const auto& o){return true;})
        .view().dump(std::cout);
}
TEST( tsv, create_frame3 )
{
    using namespace cpt::format;
    cpt::format::Tsv<> tsv;
    tsv.register_column("name");
    tsv.register_column("id");
    tsv.register_column("weight");
    tsv.register_column("measurements");

    std::vector<Person> dataset;
    for( std::size_t i (0); i < 10000000; i ++ )
    {
        Person person { 0, "Jane", 76.3, {30.1, 30.2, 30.3} };
        dataset.emplace_back(person);
    }
    {
        ::LightTuneScope<::LightTuneScopeNoPool> timer("tuple style contol");
        MCHECK("tuple style contol start", std::cout);
        std::vector<std::tuple<int, std::string, float, float>> control;
        for( auto& v : dataset)
        {
            for( auto& m : v.measurements )
            {
                std::tuple<int, std::string, float, float> tmp;
                std::get<0> ( tmp ) = v.id;
                std::get<1> ( tmp ) = v.name;
                std::get<2> ( tmp ) = v.weight;
                std::get<3> ( tmp ) = m;
                control.emplace_back(tmp);
            }
        }
        MCHECK("tuple style contol end", std::cout);
    }
    {
        ::LightTuneScope<::LightTuneScopeNoPool> timer("insert to table");
        MCHECK("insert to table start", std::cout);
        auto& id = tsv.get_meta_index("id");
        auto& name = tsv.get_meta_index("name");
        auto& weight = tsv.get_meta_index("weight");
        auto& measurements = tsv.get_meta_index("measurements");

        for( auto& v : dataset)
        {
            auto frame = tsv.create_frame();
            frame.set_property_by_idx ( id, v.id );
            frame.set_property_by_idx ( name, v.name );
            frame.set_property_by_idx ( weight, v.weight );
            frame.set_property_by_idx ( measurements, v.measurements );
            // FRAME_SET_PROPERTY( frame, v, id );
            // FRAME_SET_PROPERTY( frame, v, name );
            // FRAME_SET_PROPERTY( frame, v, weight );
            // FRAME_SET_PROPERTY( frame, v, measurements );
            frame.flush();
        }
        MCHECK("insert to table end", std::cout);
    }
    // {
    //     ::LightTuneScope<::LightTuneScopeNoPool> timer("write to file");
    //     std::ofstream fout("/tmp/killme");
    //     tsv.ref()
    //         .select("id", "name", "weight", "measurements")
    //         .where([](const auto& o){return true;})
    //         .view().dump(fout);
    // }
    tsv.clear();
    std::cout << tsv.row_num() << std::endl;
    std::cout << tsv.col_num() << std::endl;
    MCHECK("clear", std::cout);
}
