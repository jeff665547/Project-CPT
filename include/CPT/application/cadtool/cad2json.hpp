#pragma once
#include <CPT/format/converter.hpp>
#include <CPT/format/cad.hpp>
#include <CPT/format/json.hpp>
#include <boost/property_tree/ptree.hpp>
#include <CPT/utility.hpp>
#include <iostream>

namespace cpt{
namespace application{
namespace cadtool {

namespace cf = cpt::format;
namespace bpt = boost::property_tree;

struct Cad2Json : public cf::FileConverter
{
    using Cad = cf::Cad;

    virtual void operator() ( 
          const std::string& ipath
        , const std::string& opath 
    ) override 
    {
        Cad cad;
        cad.read_cad( ipath );
        auto json ( cf::make_json() );
        cad2json( cad, json );
        json.dump( std::ofstream ( opath ) );
    }

    template<class JSON>
    void cad2json( Cad& cad, JSON& json )
    {
        auto num_probesets = cad.num_probesets();

        json.add( "magic"                   , std::move( cad.magic()              ));
        json.add( "version"                 , std::move( cad.version()            ));
        json.add( "probe_array_type.name"   , std::move( cad.array_type_name()    ));
        json.add( "probe_array_type.version", std::move( cad.array_type_version() ));
        json.add( "num_probesets"           , std::move( cad.num_probesets()      ));
        json.add( "num_probes"              , std::move( cad.num_probes()         ));
        json.add( "num_features"            , std::move( cad.num_features()       ));
        json.add( "num_rows"                , std::move( cad.num_rows()           ));
        json.add( "num_cols"                , std::move( cad.num_cols()           ));
        json.add( "num_channels"            , std::move( cad.num_channels()       ));
        json.add( "max_chn_items"           , std::move( cad.max_chn_items()      ));
        json.add( "max_seq_length"          , std::move( cad.max_seq_length()     ));
        json.add( "genome_assembly"         , std::move( cad.genome_assembly()    ));
        json.add( "probe_direction"         , std::move( cad.probe_direction()    ));

        auto probeset_list = json.create_list( "probeset_list" );
        auto&& ps = cad.get_probeset_list();

        for( decltype(num_probesets) i = 0; i < num_probesets; ++i )
        {
            auto ps_node = probeset_list.create_child();

            auto&& psati = ps.at(i);
            auto&& p = psati.get_probe_list();

            auto num_probes = p.size();

            ps_node.add( "name"      , std::move( psati.name()    ));
            ps_node.add( "type"      , std::move( psati.type()    ));
            ps_node.add( "subtype"   , std::move( psati.subtype() ));
            ps_node.add( "chrom"     , std::move( psati.chrom()   ));
            ps_node.add( "start"     , std::move( psati.start()   ));
            ps_node.add( "end"       , std::move( psati.end()     ));
            ps_node.add( "strand"    , std::move( psati.strand()  ));
            ps_node.add( "desc"      , std::move( psati.desc()    ));
            ps_node.add( "num_probes", std::move( num_probes         ));
            
            auto probe_list = ps_node.create_list( "probe_list" );

            for( int j = 0; j < num_probes; ++j )
            {
                auto p_node = probe_list.create_child();

                auto&& patj = p.at(j);

                p_node.add( "probe_name", patj.probe_name() );
                p_node.add( "shape_name", patj.shape_name() );

                auto&& region_des = patj.region_des();
                auto id_node = p_node.create_list( "region_des" );

                id_node.push_back( std::move( std::get<0>( region_des )));
                id_node.push_back( std::move( std::get<1>( region_des )));
                id_node.push_back( std::move( std::get<2>( region_des )));
                id_node.push_back( std::move( std::get<3>( region_des )));

                p_node.add( "channel_des", cf::make_json( patj.channel_des() ));
                p_node.add( "sequence"   , cf::make_json( patj.sequence() ));
            }
        }
    }
};

} // cadtool
} // application
} // cpt
