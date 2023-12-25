#include <iostream>

#include <Nucleona/app/cli/gtest.hpp>
#include <CPT/format/cad.hpp>
#include <CPT/format/json.hpp>
#include <CPT/application/cadtool/json2cad.hpp>
#include <Nucleona/test/data_dir.hpp>

using namespace std;
using namespace cpt::application::cadtool;
using namespace cpt::format;

string fSample  = "-";
string fOutcad  = "-";
Json< boost::property_tree::ptree > json;
//cpt::format::cad::Cad* cad = nullptr;

// int main( int argc, char **argv )
// {
//     // abort();
// //    cad = new cpt::format::cad::Cad;
//     ::testing::InitGoogleTest( &argc, argv );
//     
//     if( argc != 3 )
//         cout << "$json2cad_test ./sample.json ./out.cad";
// 
//     fSample = argv[1];
//     fOutcad = argv[2];
//     json    = cf::read_json(fSample);
//     return RUN_ALL_TESTS();
// }
TEST ( json_test, initial )
{
    fSample = ( nucleona::test::data_dir() / "cad_example2.json" ).string();
    fOutcad = ( nucleona::test::data_dir() / "output" / "cad_example2.cad" ).string();
    if ( !boost::filesystem::exists( nucleona::test::data_dir() / "output" ) ) 
        boost::filesystem::create_directories( nucleona::test::data_dir() / "output" );
    json    = cf::read_json(fSample);

}
TEST ( json_test, header_magic )
{
    // auto json ( cf::read_json(fSample) );
    auto magic = json.get<int16_t>("magic");
    if(magic!= 113)
        FAIL();
}
TEST ( json_test, header_rows )
{
    auto rows = json.get_optional<uint16_t>("num_rows").value_or(0u);
    if(rows!= 6)
        FAIL();
}
TEST ( json_test, header_cols )
{
    auto cols = json.get_optional<uint16_t>("num_cols").value_or(0u);
    if(cols!= 6)
        FAIL();
}
TEST ( json_test, header_max_channel_items )
{
    auto val = json.get_optional<uint16_t>("max_chn_items").value_or(0u);
    if(val!= 4)
        FAIL();
}
TEST ( json_test, header_max_sequence_length )
{
    auto val = json.get_optional<uint16_t>("max_seq_length").value_or(0u);
    if(val!= 40)
        FAIL();
}
TEST ( json2cad_test, j2c_converter )
{
    Json2Cad *pJ2C = new Json2Cad();
    pJ2C->operator()(fSample, fOutcad);
    delete pJ2C;
}
TEST ( cad_test, header_fields )
{
    cad::Cad cad;
    cad.read_cad(fOutcad);

    auto&& magic = cad.magic();
    if(magic!= 113)
      ADD_FAILURE() << "Error: the \"magic\" is " << magic;
    
    auto&& ci0 = json.get_optional<uint16_t>("max_chn_items").value_or(0u);
    auto&& ci1 = cad.max_chn_items();
    if(ci0 != ci1)
      ADD_FAILURE() << "Error: the \"max_chn_items\" is " << ci0 << ":" << ci1;

    auto&& sl0 = json.get_optional<uint16_t>("max_seq_length").value_or(0u);
    auto&& sl1 = cad.max_seq_length();
    if(sl0 != sl1)
      ADD_FAILURE() << "Error: the \"max_seq_length\" is " << sl0 << ":" << sl1;

    auto&& pd0 = json.get_optional<std::string>("probe_direction");
    auto&& pd1 = cad.probe_direction();
    if(pd0 != pd1)
      ADD_FAILURE() << "Error: the \"probe_direction\" is " << pd0 << ":" << pd1;

    auto&& size0 = cad.num_probesets();
    auto&& size1 = json.get_list( "probeset_list" ).size();
    if(size0 != size1)
      ADD_FAILURE() << "Error: size of \"probeset_list\" is " << size0 << ":" << size1;

    auto&& cadProbesetList = cad.get_probeset_list();
    auto&& pbs0 = cadProbesetList.at(0);
    auto&& pbs1 = cadProbesetList.at(1);

    auto&& pbs0_name = pbs0.name();
    if(pbs0_name != "PS00AC")
      ADD_FAILURE() << "Error: the pbs0.name is " << pbs0_name << " (PS00AC)";

    auto&& pbs0_start = pbs0.start();
    auto&& pbs0_end   = pbs0.end();
    if(pbs0_start != 6)
      ADD_FAILURE() << "Error: the pbs0.start is " << pbs0_start << " (6)";
    if(pbs0_end != 7)
      ADD_FAILURE() << "Error: the pbs0.end is " << pbs0_end << " (7)";

    auto&& pbs1_name = pbs1.name();
    if(pbs1_name != "PS01AC")
      ADD_FAILURE() << "Error: the ps1.name is " << pbs1_name << " (PS01AC)";

    auto&& pbs1_start = pbs1.start();
    auto&& pbs1_end   = pbs1.end();
    if(pbs1_start != 40)
      ADD_FAILURE() << "Error: the pbs1.start is " << pbs1_start << " (40)";
    if(pbs1_end != 200)
      ADD_FAILURE() << "Error: the pbs1.end is " << pbs1_end << " (200)";

}

TEST ( cad_test, compare_probeset )
{
    cad::Cad cad;
    cad.read_cad(fOutcad);

    auto&& cadProbesetList = cad.get_probeset_list();
    auto&& c_set_size = cad.num_probesets();
    auto&& j_set_size = json.get< uint32_t >( "num_probesets" );
    if(c_set_size != j_set_size)
      ADD_FAILURE() << "the \"size\" of probeset is not the same (" 
                    << c_set_size << "," << j_set_size << ")";

    uint32_t i = 0;
    for(auto&& iElem : json.get_list( "probeset_list" ))
    {
      auto&& cadProbeset = cadProbesetList.at(i);
      auto&& c_name       = cadProbeset.name();
      auto&& j_name       = iElem.second.get<std::string>("name");
      if(c_name != j_name)
        ADD_FAILURE() << "the \"name\" of probeset[ " << i << " ] is not the same.";

      uint32_t j = 0;
      auto&& jsonProbeset     = cf::make_json( iElem.second );
      auto&& cadProbesetList  = cadProbeset.get_probe_list();
      for( auto&& jElem : jsonProbeset.get_list( "probe_list" )) 
      {
          auto&& cadProbe  = cadProbesetList.at(j);
          auto&& c_pb_name = cadProbe.probe_name();
          auto&& j_pb_name = jElem.second.get< uint32_t >( "probe_name" );
          if(c_pb_name != j_pb_name)
            ADD_FAILURE() << "Probe Name: [" << i <<","<< j <<" ] is not the same.";

          auto&& c_sh_name = cadProbe.shape_name();
          auto&& j_sh_name = jElem.second.get< uint16_t >( "shape_name" );
          if(c_pb_name != j_pb_name)
            ADD_FAILURE() << "Shape Name: [" << i <<","<< j <<" ] is not the same.";




          j++;
      }

      i++;
    }

}
