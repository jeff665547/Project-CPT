// TODO file logically test, for detail col test
#include <mlpack/core.hpp>
#include <vector>
#include <iostream>
#include <CPT/format/json.hpp>
#include <CPT/application/cadtool/cad2json.hpp>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <Nucleona/test/data_dir.hpp>

namespace cf = cpt::format;
namespace bpt = boost::property_tree;

cf::Cad* cad = new cf::Cad;
cf::Json< boost::property_tree::ptree > json( cf::make_json() );

std::string cadin;
// #ifdef USE_CI_MAIN
#include <Nucleona/app/cli/gtest.hpp>
TEST( cad2json_test, initial )
{
    cadin = ( nucleona::test::data_dir() / "example.cad" ).string();
} 
//#else
// int main( int argc, char **argv )
// {
//     ::testing::InitGoogleTest( &argc, argv );
// 
//     if( argc != 2 )
//         throw std::runtime_error( "./cad2json_test in_cad_file" );
// 
//     cadin = argv[1];
// 
//     return RUN_ALL_TESTS();
// }
// #endif

// TEST( CAD_format, min_example_create )
// {
//     cf::Cad ccad; 
//     ccad.magic( 113 );
//     ccad.create_cad( cadin, 2, 4 );
//     ccad.version( "000" );
//     ccad.array_type_version( "0" );
//     ccad.array_type_name( "min_example" );
//     ccad.num_rows( 6 );
//     ccad.num_cols( 6 );
//     ccad.num_channels( 2 );
//     ccad.max_seq_length( 40 );
//     ccad.genome_assembly( "hg19" );
//     ccad.probe_direction( "3-5" );
//     auto&& psl = ccad.get_probeset_list();
//     {
//         auto&& ps( psl.create_probeset( 2 ));
//         ps.name( "PS01AC" );
//         ps.type( "Genotyping" );
//         ps.subtype( "LigationBased" );
//         ps.chrom( "chrS" );
//         ps.start( 6 );
//         ps.end( 7 );
//         ps.strand( '+' );
//         ps.desc( "AC" );
//         auto&& pl = ps.get_probe_list();
//         {
//             auto&& p = pl.create_probe();
//             p.probe_name( 7 );
//             p.shape_name( 1 );
//             p.region_des({ 1, 1, 1, 1 });
// 
//             auto cd = cf::make_json();
//             cd.add( "allele" , "A/B" );
//             cd.add( "base"   , "T/G" );
//             cd.add( "channel", "1/0" );
//             p.channel_des( cd.root );
// 
// 
//             auto sq = cf::make_json();
//             sq.add( "start"  , 2 );
//             sq.add( "length" , 4 );
//             sq.add( "strand" , '-' );
//             sq.add( "content", "GTCA" );
//             p.sequence( sq.root );
//         }
//         {
//             auto&& p = pl.create_probe();
//             p.probe_name( 8 );
//             p.shape_name( 2 );
//             p.region_des({ 1, 2, 2, 1 });
// 
//             auto cd = cf::make_json();
//             cd.add( "allele" , "A/B" );
//             cd.add( "base"   , "T/G" );
//             cd.add( "channel", "1/0" );
//             p.channel_des( cd.root );
// 
// 
//             auto sq = cf::make_json();
//             sq.add( "start"  , 2 );
//             sq.add( "length" , 4 );
//             sq.add( "strand" , '-' );
//             sq.add( "content", "GTCA" );
//             p.sequence( sq.root );
//         }
//     }
//     {
//         auto&& ps( psl.create_probeset( 2 ));
//         ps.name( "PS02AC" );
//         ps.type( "Genotyping" );
//         ps.subtype( "LigationBased" );
//         ps.chrom( "chrM" );
//         ps.start( 9 );
//         ps.end( 20 );
//         ps.strand( '+' );
//         ps.desc( "AC" );
//         auto&& pl = ps.get_probe_list();
//         {
//             auto&& p = pl.create_probe();
//             p.probe_name( 10 );
//             p.shape_name( 3 );
//             p.region_des({ 3, 2, 2, 3 });
// 
//             auto cd = cf::make_json();
//             cd.add( "allele" , "A/B" );
//             cd.add( "base"   , "T/G" );
//             cd.add( "channel", "1/0" );
//             p.channel_des( cd.root );
// 
// 
//             auto sq = cf::make_json();
//             sq.add( "start"  , 2 );
//             sq.add( "length" , 4 );
//             sq.add( "strand" , '-' );
//             sq.add( "content", "GTCA" );
//             p.sequence( sq.root );
//         }
//         {
//             auto&& p = pl.create_probe();
//             p.probe_name( 4 );
//             p.shape_name( 6 );
//             p.region_des({ 4, 4, 4, 4 });
// 
//             auto cd = cf::make_json();
//             cd.add( "allele" , "A/B" );
//             cd.add( "base"   , "T/G" );
//             cd.add( "channel", "1/0" );
//             p.channel_des( cd.root );
// 
// 
//             auto sq = cf::make_json();
//             sq.add( "start"  , 2 );
//             sq.add( "length" , 4 );
//             sq.add( "strand" , '-' );
//             sq.add( "content", "GTCA" );
//             p.sequence( sq.root );
//         }
//     }
// }

TEST( cad2json_test, read_cad )
{
    cad->read_cad( cadin );
}

TEST ( cad2json_test, magic_check )
{
    auto&& magic = cad->magic();

    if( magic != 113 )
        FAIL();
}

TEST( cad2json_test, version_check )
{
    auto&& version = cad->version();

    if( version == "" )
        FAIL();
}

TEST( cad2json_test, probe_array_type_name_check )
{
    auto&& probe_array_type_name = cad->array_type_name();

    if( probe_array_type_name == "" )
        FAIL();
}

TEST( cad2json_test, probe_array_type_version_check )
{
    auto&& probe_array_type_version = cad->array_type_version();

    if( probe_array_type_version == "" )
        FAIL();
}

TEST( cad2json_test, num_probes_check )
{
    auto&& num_probes = cad->num_probes();

    if( num_probes < 0 || num_probes > 4294967295 || num_probes != num_probes || ( num_probes != 0 && num_probes == '\0' ))
        FAIL();
}

TEST( cad2json_test, num_probesets_check )
{
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets < 0 || num_probesets > 4294967295 || num_probesets != num_probesets || ( num_probesets != 0 && num_probesets == '\0' ))
        FAIL();
}

TEST( cad2json_test, num_features_check )
{
    auto&& num_features = cad->num_features();

    if( num_features < 0 || num_features > 4294967295 || num_features != num_features || ( num_features != 0 && num_features == '\0' ))
        FAIL();
}

TEST( cad2json_test, num_rows_check )
{
    auto&& num_rows = cad->num_rows(); 

    if( num_rows < 0 || num_rows > 65535 || num_rows != num_rows || ( num_rows != 0 && num_rows == '\0' ))
        FAIL();
}

TEST( cad2json_test, num_cols_check )
{
    auto&& num_cols = cad->num_cols();

    if( num_cols < 0 || num_cols > 65535 || num_cols != num_cols || ( num_cols != 0 && num_cols == '\0' ))
        FAIL();
}

TEST( cad2json_test, num_channels_check )
{
    auto&& num_channels = cad->num_channels();

    if( num_channels < 0 || num_channels > 65535 || num_channels != num_channels || ( num_channels != 0 && num_channels == '\0' ))
        FAIL();
}

TEST( cad2json_test, max_seq_length_check )
{
    auto&& max_seq_length = cad->max_seq_length();

    if( max_seq_length < 0 || max_seq_length > 65535 || max_seq_length != max_seq_length || ( max_seq_length != 0 && max_seq_length == '\0' ))
        FAIL();
}

TEST( cad2json_test, max_chn_items_check )
{
    auto&& max_chn_items = cad->max_chn_items();

    if( max_chn_items < 0 || max_chn_items > 65535 || max_chn_items != max_chn_items || ( max_chn_items != 0 && max_chn_items == '\0' ))
        FAIL();
}

TEST( cad2json_test, genome_assembly_check )
{
    auto&& genome_assembly = cad->genome_assembly();

    if( genome_assembly == "" )
        FAIL();
}

TEST( cad2json_test, probe_direction_check )
{
    bool check( true );
    auto&& probe_direction = cad->probe_direction();

    if( probe_direction != "3-5" && probe_direction != "5-3" )
        check = false;

    if( probe_direction == "" || !check )
        FAIL();
}

TEST( cad2json_test, probeset_list_check )
{
    auto&& is_probeset_empty = cad->is_probeset_empty();

    if( is_probeset_empty )
        FAIL();
}

TEST( cad2json_test, probeset_list_name_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& name = psati.name();

        if( name == "" )
            ADD_FAILURE() << "the \"name\" of probeset[ " << i << " ] is empty";
    }
}

TEST( cad2json_test, probeset_list_type_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& type = psati.type();

        if( type == "" ||( type != "Expression" && type != "Copynumber" && type != "Genotyping" && type != "Sequencing" ))
            ADD_FAILURE() << "the \"tpye\" of probeset[ " << i << " ] is empty";
    }
}

TEST( cad2json_test, probeset_list_subtype_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& subtype = psati.subtype();

        if( subtype == "" ||( subtype != "TagBased" && subtype != "LigationBased" && subtype != "PolymeraseExtensionBased" && subtype != "Sequencing" ))
            ADD_FAILURE() << "the \"subtpye\" of probeset[ " << i << " ] is empty";
    }
}

TEST( cad2json_test, probeset_list_chrom_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& chrom = psati.chrom();

        if( chrom == "" || ( chrom != "" && chrom != "---" && chrom.substr( 0, 3 ) != "chr" ))
            ADD_FAILURE() << "the \"chrom\" of probeset[ " << i << " ] has wrong prefix: \"" << chrom.substr( 0, 3 ) << " \", it should be \"chr\"";
    }
}

TEST( cad2json_test, probeset_list_start_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& start = psati.start();
        auto&& end   = psati.end();

        if( start < 0 || start > 4294967295 || start != start || ( start != 0 && start == '\0' ))
            ADD_FAILURE() << "the \"start\" of probeset[ " << i << " ] is wrong";

        if( start != 0 && start == end )
            ADD_FAILURE() << "the \"start\" of probeset[ " << i << " ] is 0 base, not 1 base";
    }
}

TEST( cad2json_test, probeset_list_end_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& start = psati.start();
        auto&& end   = psati.end();

        if( end < 0 || end > 4294967295 || end != end || ( end != 0 && end == '\0' ))
            ADD_FAILURE() << "the \"end\" of probeset[ " << i << " ] is wrong";

        if( end != 0 && end == start )
            ADD_FAILURE() << "the \"end\" of probeset[ " << i << " ] is 0 base, not 1 base";
    }
}

TEST( cad2json_test, probeset_list_strand_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& strand = psati.strand();

        if( strand != '.' && strand != '-' && strand != '+' )
            ADD_FAILURE() << "the \"strand\" of probeset[ " << i << " ] is wrong";
    }
}

TEST( cad2json_test, probeset_list_desc_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& type = psati.type();
        auto&& desc = psati.desc();

        if( desc == "" || ( type == "Genotyping" && desc == "---" ))
            ADD_FAILURE() << "the \"desc\" of probeset[ " << i << " ] can't be empty with \"type: Genotyping\"";
    }
}

TEST( cad2json_test, probeset_list_probe_list_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& p = psati.get_probe_list();

        if( p.size() == 0 )
            ADD_FAILURE() << "the \"probe_list\" of probeset[ " << i << " ] is empty";
    }
}

TEST( cad2json_test, probeset_list_probe_list_probe_name_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& p = psati.get_probe_list();

        for( int j = 0; j < p.size(); ++j )
        {
            auto&& patj = p.at(j);
            auto&& probe_name = patj.probe_name();

            if( probe_name < 0 || probe_name > 4294967295 || probe_name != probe_name || ( probe_name != 0 && probe_name == '\0' ))
                ADD_FAILURE() << "the \"probe_name\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
        }
    }
}

TEST( cad2json_test, probeset_list_probe_list_shape_name_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& p = psati.get_probe_list();

        for( int j = 0; j < p.size(); ++j )
        {
            auto&& patj = p.at(j);
            auto&& shape_name = patj.shape_name();

            if( shape_name < 0 || shape_name > 65535 || shape_name != shape_name || ( shape_name != 0 && shape_name == '\0' ))
                ADD_FAILURE() << "the \"shape_name\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
        }
    }
}

TEST( cad2json_test, probeset_list_probe_list_region_des_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& p = psati.get_probe_list();

        for( int j = 0; j < p.size(); ++j )
        {
            auto&& patj = p.at(j);
            auto&& region_des = patj.region_des();

            if( std::get<0>( region_des ) < 0 || std::get<0>( region_des ) > 4294967295 || std::get<0>( region_des ) != std::get<0>( region_des ) || ( std::get<0>( region_des ) != 0 && std::get<0>( region_des ) == '\0' ))
                ADD_FAILURE() << "the \"std::get<0>( region_des )\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";

            if( std::get<1>( region_des ) < 0 || std::get<1>( region_des ) > 4294967295 || std::get<1>( region_des ) != std::get<1>( region_des ) || ( std::get<1>( region_des ) != 0 && std::get<1>( region_des ) == '\0' ))
                ADD_FAILURE() << "the \"std::get<1>( region_des )\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";

            if( std::get<2>( region_des ) < 0 || std::get<2>( region_des ) > 65535      || std::get<2>( region_des ) != std::get<2>( region_des ) || ( std::get<2>( region_des ) != 0 && std::get<2>( region_des ) == '\0' ))
                ADD_FAILURE() << "the \"std::get<2>( region_des )\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";

            if( std::get<3>( region_des ) < 0 || std::get<3>( region_des ) > 65535      || std::get<3>( region_des ) != std::get<3>( region_des ) || ( std::get<3>( region_des ) != 0 && std::get<3>( region_des ) == '\0' ))
                ADD_FAILURE() << "the \"std::get<3>( region_des )\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
        }
    }
}

TEST( cad2json_test, probeset_list_probe_list_channel_des_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& p = psati.get_probe_list();

        for( int j = 0; j < p.size(); ++j )
        {
            auto&& patj = p.at(j);

            try {
                auto&& allele_str = patj.channel_des().get< std::string >( "allele" );

                if( allele_str == "" )
                {
                    ADD_FAILURE() << "the \"allele\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                }
                else
                {
                    std::vector< std::string > alleles;
                    boost::iter_split( alleles, allele_str, boost::algorithm::first_finder( "/" ));

                    if( alleles.empty() && allele_str != "---" )
                    {
                        ADD_FAILURE() << "the \"allele\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                    }
                    else
                    {
                        for( auto&& allele : alleles )
                        {
                            if( allele != "A" && allele != "B" && allele != "C" && allele != "D" )
                                ADD_FAILURE() << "the \"allele " << allele << "\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                        }
                    }
                }
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"allele\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }

            try {
                auto&& channel_str = patj.channel_des().get< std::string >( "channel" );

                if( channel_str == "" )
                {
                    ADD_FAILURE() << "the \"channel\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                }
                else
                {
                    std::vector< std::string > channels;
                    boost::iter_split( channels, channel_str, boost::algorithm::first_finder( "/" ));

                    if( channels.empty() && channel_str != "---" )
                    {
                        ADD_FAILURE() << "the \"channel\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                    }
                    else
                    {
                        for( auto&& channel_ : channels )
                        {
                            auto&& channel = std::stoi( channel_ );

                            if( channel < -128 || channel > 127 || channel != channel )
                                ADD_FAILURE() << "the \"channel " << channel << "\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                        }
                    }
                }
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"channel\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }

            try {
                auto&& base_str = patj.channel_des().get< std::string >( "base" );

                if( base_str == "" )
                {
                    ADD_FAILURE() << "the \"base\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                }
                else
                {
                    std::vector< std::string > bases;
                    boost::iter_split( bases, base_str, boost::algorithm::first_finder( "/" ));

                    if( bases.empty() && base_str != "---" )
                    {
                        ADD_FAILURE() << "the \"base\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                    }
                    else
                    {
                        for( auto&& base : bases )
                        {
                            if( base != "A" && base != "C" && base != "G" && base != "T" && base != "N" )
                                ADD_FAILURE() << "the \"base " << base << "\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                        }
                    }
                }
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"base\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }
        }
    }
}

TEST( cad2json_test, probeset_list_probe_list_sequence_check )
{
    auto&& ps = cad->get_probeset_list();
    auto&& num_probesets = cad->num_probesets();

    if( num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();

    for( int i = 0; i < num_probesets; ++i )
    {
        auto&& psati = ps.at(i);
        auto&& p = psati.get_probe_list();

        for( int j = 0; j < p.size(); ++j )
        {
            auto&& patj = p.at(j);

            try {
                auto&& start = patj.sequence().get< uint32_t >( "start" );

                if( start < 0 || start > 4294967295 || start != start || ( start != 0 && start == '\0' ))
                    ADD_FAILURE() << "the \"start\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"start\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }

            try {
                auto&& length = patj.sequence().get< uint16_t >( "length" );

                if( length < 0 || length > 65535 || length != length || ( length != 0 && length == '\0' ))
                    ADD_FAILURE() << "the \"length\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"length\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }

            try {
                auto&& strand = patj.sequence().get< char >( "strand" );

                if( strand != '.' && strand != '-' && strand != '+' )
                    ADD_FAILURE() << "the \"strand\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"strand\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }

            try {
                auto&& content = patj.sequence().get< std::string >( "content" );

                if( content == "" )
                    ADD_FAILURE() << "the \"content\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"content\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is need";
            }
        }
    }
}

TEST( cad2json_test, cad_json_conversion )
{
    if( cad->magic() == '\0' )
        FAIL();

    std::unique_ptr< cpt::application::cadtool::Cad2Json > converter (
        new cpt::application::cadtool::Cad2Json
    );

    converter->cad2json( *cad, json );
}

TEST( cad2json_test, magic_compare )
{
    auto&& magic = json.get< int16_t >( "magic" );
    auto&& cad_magic = cad->magic();

    if( cad_magic != magic || magic != 113 )
        FAIL();
}

TEST( cad2json_test, version_compare )
{
    auto&& version = json.get< std::string >( "version" );
    auto&& cad_version = cad->version();

    if( cad_version != version || version == "" )
        FAIL();
}

TEST( cad2json_test, array_type_name_compare )
{
    auto&& array_type_name = json.get< std::string >( "probe_array_type.name" );
    auto&& cad_array_type_name = cad->array_type_name();

    if( cad_array_type_name != array_type_name || array_type_name == "" )
        FAIL();
}

TEST( cad2json_test, array_type_version_compare )
{
    auto&& array_type_version = json.get< std::string >( "probe_array_type.version" );
    auto&& cad_array_type_version = cad->array_type_version();

    if( cad_array_type_version != array_type_version || array_type_version == "" )
        FAIL();
}

TEST( cad2json_test, num_rows_compare )
{
    auto&& num_rows = json.get< uint16_t >( "num_rows" );
    auto&& cad_num_rows = cad->num_rows();

    if( cad_num_rows != num_rows || num_rows == '\0' )
        FAIL();
}

TEST( cad2json_test, num_cols_compare )
{
    auto&& num_cols = json.get< uint16_t >( "num_cols" );
    auto&& cad_num_cols = cad->num_cols();

    if( cad_num_cols != num_cols || num_cols == '\0' )
        FAIL();
}

TEST( cad2json_test, num_probes_compare )
{
    auto&& num_probes = json.get< uint32_t >( "num_probes" );
    auto&& cad_num_probes = cad->num_probes();

    if( cad_num_probes != num_probes || num_probes == '\0' )
        FAIL();
}

TEST( cad2json_test, num_probesets_compare )
{
    auto&& num_probesets = json.get< uint32_t >( "num_probesets" );
    auto&& cad_num_probesets = cad->num_probesets();

    if( cad_num_probesets != num_probesets || num_probesets == '\0' )
        FAIL();
}

TEST( cad2json_test, num_features_compare )
{
    auto&& num_features = json.get< uint32_t >( "num_features" );
    auto&& cad_num_features = cad->num_features();

    if( cad_num_features != num_features || num_features == '\0' )
        FAIL();
}

TEST( cad2json_test, num_channels_compare )
{
    auto&& num_channels = json.get< uint16_t >( "num_channels" );
    auto&& cad_num_channels = cad->num_channels();

    if( cad_num_channels != num_channels || num_channels == '\0' )
        FAIL();
}

TEST( cad2json_test, max_chn_items_compare )
{
    auto&& max_chn_items = json.get< uint16_t >( "max_chn_items" );
    auto&& cad_max_chn_items = cad->max_chn_items();

    if( cad_max_chn_items != max_chn_items )
        FAIL();
}

TEST( cad2json_test, max_seq_length_compare )
{
    auto&& max_seq_length = json.get< uint16_t >( "max_seq_length" );
    auto&& cad_max_seq_length = cad->max_seq_length();

    if( cad_max_seq_length != max_seq_length || max_seq_length == '\0' )
        FAIL();
}

TEST( cad2json_test, genome_assembly_compare )
{
    auto&& genome_assembly = json.get< std::string >( "genome_assembly" );
    auto&& cad_genome_assembly = cad->genome_assembly();

    if( cad_genome_assembly != genome_assembly || genome_assembly == "" )
        FAIL();
}

TEST( cad2json_test, probe_direction_compare )
{
    auto&& probe_direction = json.get< std::string >( "probe_direction" );
    auto&& cad_probe_direction = cad->probe_direction();

    if( cad_probe_direction != probe_direction || probe_direction == "" )
        FAIL();
}

TEST( cad2json_test, probeset_list_name_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& name = ps.second.get< std::string >( "name" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_name = cad_psati.name();

        if( cad_name != name )
            ADD_FAILURE() << "the \"name\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_type_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& type = ps.second.get< std::string >( "type" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_type = cad_psati.type();

        if( cad_type != type )
            ADD_FAILURE() << "the \"type\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_subtype_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& subtype = ps.second.get< std::string >( "subtype" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_subtype = cad_psati.subtype();

        if( cad_subtype != subtype )
            ADD_FAILURE() << "the \"subtype\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_chrom_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& chrom = ps.second.get< std::string >( "chrom" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_chrom = cad_psati.chrom();

        if( cad_chrom != chrom )
            ADD_FAILURE() << "the \"chrom\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_start_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& start = ps.second.get< uint32_t >( "start" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_start = cad_psati.start();

        if( cad_start != start )
            ADD_FAILURE() << "the \"start\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_end_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& end = ps.second.get< uint32_t >( "end" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_end = cad_psati.end();

        if( cad_end != end )
            ADD_FAILURE() << "the \"end\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_strand_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& strand = ps.second.get< char >( "strand" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_strand = cad_psati.strand();

        if( cad_strand != strand )
            ADD_FAILURE() << "the \"strand\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_desc_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        auto&& desc = ps.second.get< std::string >( "desc" );

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_desc = cad_psati.desc();

        if( cad_desc != desc )
            ADD_FAILURE() << "the \"desc\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_probe_list_probe_name_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        size_t j = 0;

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_p = cad_psati.get_probe_list();
        auto&& jps = cf::make_json( ps.second );

        for( auto&& p : jps.get_list( "probe_list" )) 
        {
            auto&& probe_name = p.second.get< uint32_t >( "probe_name" );

            auto&& cad_patj = cad_p.at(j);
            auto&& cad_probe_name = cad_patj.probe_name();

            if( cad_probe_name != probe_name )
                ADD_FAILURE() << "the \"probe_name\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";

            ++j;
        }

        if( j == 0 && cad_p.size() != 0 )
            ADD_FAILURE() << "the \"probe_list\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_probe_list_shape_name_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        size_t j = 0;

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_p = cad_psati.get_probe_list();
        auto&& jps = cf::make_json( ps.second );

        for( auto&& p : jps.get_list( "probe_list" )) 
        {
            auto&& shape_name = p.second.get< uint32_t >( "shape_name" );

            auto&& cad_patj = cad_p.at(j);
            auto&& cad_shape_name = cad_patj.shape_name();

            if( cad_shape_name != shape_name )
                ADD_FAILURE() << "the \"shape_name\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";

            ++j;
        }

        if( j == 0 && cad_p.size() != 0 )
            ADD_FAILURE() << "the \"probe_list\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_probe_list_region_des_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        size_t j = 0;

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_p = cad_psati.get_probe_list();
        auto&& jps = cf::make_json( ps.second );

        for( auto&& p : jps.get_list( "probe_list" )) 
        {
            size_t k = 0;

            auto&& cad_patj = cad_p.at(j);
            auto&& cad_region_des = cad_patj.region_des();
            auto&& jp = cf::make_json( p.second );

            for( auto&& pt : jp.get_list( "region_des" )) 
            {
                if( k == 0 )
                {
                    auto&& region_des = pt.second.get_value< uint32_t >();

                    if( std::get<0>( cad_region_des ) != region_des )
                        ADD_FAILURE() << "the \"region_des[ " << k << " ]\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                }

                if( k == 1 )
                {
                    auto&& region_des = pt.second.get_value< uint32_t >();

                    if( std::get<1>( cad_region_des ) != region_des )
                        ADD_FAILURE() << "the \"region_des[ " << k << " ]\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                }

                if( k == 2 )
                {
                    auto&& region_des = pt.second.get_value< uint16_t >();

                    if( std::get<2>( cad_region_des ) != region_des )
                        ADD_FAILURE() << "the \"region_des[ " << k << " ]\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                }

                if( k == 3 )
                {
                    auto&& region_des = pt.second.get_value< uint16_t >();

                    if( std::get<3>( cad_region_des ) != region_des )
                        ADD_FAILURE() << "the \"region_des[ " << k << " ]\" of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
                }

                ++k;
            }

            ++j;
        }

        if( j == 0 && cad_p.size() != 0 )
            ADD_FAILURE() << "the \"probe_list\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_probe_list_channel_des_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        size_t j = 0;

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_p = cad_psati.get_probe_list();

        auto&& jps = cf::make_json( ps.second );

        for( auto&& p : jps.get_list( "probe_list" )) 
        {
            auto&& cad_patj = cad_p.at(j);
            auto&& cad_channel_des = cad_patj.channel_des();

            auto&& cad_allele  = cad_channel_des.get< std::string >( "allele" );
            auto&& cad_channel = cad_channel_des.get< std::string >( "channel" );
            auto&& cad_base    = cad_channel_des.get< std::string >( "base" );

            auto&& jp = cf::make_json( p.second );
            auto&& channel_des = jp.get_child( "channel_des" );

            try {
                auto&& allele = channel_des.get< std::string >( "allele" );

                if( cad_allele != allele )
                    ADD_FAILURE() << "the \"allele\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"allele\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }

            try {
                auto&& channel = channel_des.get< std::string >( "channel" );

                if( cad_channel != channel )
                    ADD_FAILURE() << "the \"channel\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"channel\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }

            try {
                auto&& base = channel_des.get< std::string >( "base" );

                if( cad_base != base )
                    ADD_FAILURE() << "the \"base\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"base\" of channel_des of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }

            ++j;
        }

        if( j == 0 && cad_p.size() != 0 )
            ADD_FAILURE() << "the \"probe_list\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, probeset_list_probe_list_sequence_compare )
{
    size_t i = 0;
    auto&& cad_ps = cad->get_probeset_list();

    for( auto&& ps : json.get_list( "probeset_list" ))
    {
        size_t j = 0;

        auto&& cad_psati = cad_ps.at(i);
        auto&& cad_p = cad_psati.get_probe_list();

        auto&& jps = cf::make_json( ps.second );

        for( auto&& p : jps.get_list( "probe_list" )) 
        {
            auto&& cad_patj = cad_p.at(j);
            auto&& cad_sequence = cad_patj.sequence();

            auto&& cad_start = cad_sequence.get< uint32_t >( "start" );
            auto&& cad_length = cad_sequence.get< uint16_t >( "length" );
            auto&& cad_strand = cad_sequence.get< char >( "strand" );
            auto&& cad_content = cad_sequence.get< std::string >( "content" );

            auto&& jp = cf::make_json( p.second );
            auto&& sequence = jp.get_child( "sequence" );

            try {
                auto&& start = sequence.get< uint32_t >( "start" );

                if( cad_start != start )
                    ADD_FAILURE() << "the \"start\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"start\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }

            try {
                auto&& length = sequence.get< uint16_t >( "length" );

                if( cad_length != length )
                    ADD_FAILURE() << "the \"length\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"length\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }

            try {
                auto&& strand = sequence.get< char >( "strand" );

                if( cad_strand != strand )
                    ADD_FAILURE() << "the \"strand\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"strand\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }

            try {
                auto&& content = sequence.get< std::string >( "content" );

                if( cad_content != content )
                    ADD_FAILURE() << "the \"content\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }
            catch( std::exception const& e ) {
                ADD_FAILURE() << "the \"content\" of sequence of probe[ " << j << " ] of probeset[ " << i << " ] is wrong";
            }

            ++j;
        }

        if( j == 0 && cad_p.size() != 0 )
            ADD_FAILURE() << "the \"probe_list\" of probeset[ " << i << " ] is wrong";

        ++i;
    }

    if( i == 0 && cad_ps.size() != 0 )
        FAIL();
}

TEST( cad2json_test, write_cad )
{
    delete cad;
}

TEST( cad2json_test, print_json )
{
    json.dump( std::cerr );
}
