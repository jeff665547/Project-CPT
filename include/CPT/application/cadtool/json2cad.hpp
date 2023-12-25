#pragma once
#include <CPT/format/converter.hpp>
#include <CPT/format/cad.hpp>
#include <CPT/format/json.hpp>

#include <vector>
#include <cassert>
#include <boost/archive/text_oarchive.hpp>
#include <iostream>
#include <fstream>
namespace cpt{
namespace application{
namespace cadtool {
namespace cf = cpt::format;

struct Json2Cad : public cf::FileConverter
{
    template<class PTREE>
    uint32_t get_probe_id ( cf::Json<PTREE>& jpnode )
    {
        auto jshape_id ( jpnode.template get_optional<uint32_t>("probe_id") );
        return jshape_id.get();
    }

    template<class PTREE>
    uint16_t get_shape_id ( cf::Json<PTREE>& jpnode )
    {
        auto jshape_id ( jpnode.template get_optional<uint16_t>("shape_id") );
        return jshape_id.get();
    }

    virtual void operator()( const std::string& ipath, const std::string& opath ) override
    {
        auto json ( cf::read_json(std::ifstream( ipath )) );
        cf::Cad cad;
        cad.create_cad(
          opath,
          json.get<uint32_t>("num_probesets"),
          json.get<uint32_t>("num_features")
        );
        cad.magic( json.get<int16_t>("magic") );
        cad.version( json.get<std::string>("version") );
        cad.array_type_name( json.get<std::string>("probe_array_type.name") );
        cad.array_type_version( json.get<std::string>("probe_array_type.version") );
        cad.num_rows( json.get<uint16_t>("num_rows") );
        cad.num_cols( json.get<uint16_t>("num_cols") );
        cad.num_channels( json.get<uint16_t>("num_channels")  );
        cad.max_chn_items( json.get<uint16_t>("max_chn_items") );
        cad.max_seq_length( json.get<uint16_t>("max_seq_length") );
        cad.genome_assembly( json.get<std::string>("genome_assembly") );
        cad.probe_direction( json.get<std::string>("probe_direction") );

        //uint32_t i = 0;
        auto&& cadProbesetList = cad.get_probeset_list();
        for(auto&& iElem : json.get_list( "probeset_list" ))
        {
            auto&& je ( cf::make_json( iElem.second ));
            auto&& plsize ( je.get_list("probe_list").size() );
            uint32_t n0 = plsize;
            auto&& cadProbeset ( cadProbesetList.create_probeset(n0) );
            cadProbeset.name(    iElem.second.get<std::string>("name") );
            cadProbeset.type(    iElem.second.get<std::string>("type") );
            cadProbeset.subtype( iElem.second.get<std::string>("subtype") );
            cadProbeset.chrom(   iElem.second.get<std::string>("chrom") );
            cadProbeset.start(   iElem.second.get<uint32_t>("start") );
            cadProbeset.end(     iElem.second.get<uint32_t>("end") );
            cadProbeset.strand(  iElem.second.get<char>("strand") );
            cadProbeset.desc(    iElem.second.get<std::string>("desc") );


            //uint32_t j = 0;
            auto&& jsonProbeset     = cf::make_json( iElem.second );
            auto&& cadProbesetList  = cadProbeset.get_probe_list();
            for( auto&& jElem : jsonProbeset.get_list( "probe_list" )) 
            {
                auto&& cadProbe  = cadProbesetList.create_probe();
                cadProbe.probe_name( jElem.second.get< uint32_t >( "probe_name" ) );
                cadProbe.shape_name( jElem.second.get< uint16_t >( "shape_name" ) );
 
                size_t k0, k1, k2, k3, k = 0;
                auto&& jsonProbe = cf::make_json( jElem.second );
                for( auto&& pt : jsonProbe.get_list( "region_des" ))
                {
                  if( k==0 ) {
                    k0 = pt.second.get_value<uint32_t>();
                  }
                  else if( k==1 ) {
                    k1 = pt.second.get_value<uint32_t>();
                  }
                  else if( k==2 ) {
                    k2 = pt.second.get_value<uint16_t>();
                  }
                  else if( k==3 ) {
                    k3 = pt.second.get_value<uint16_t>();
                  }
                  k++;
                }
                cadProbe.region_des( {k0, k1, k2, k3 });

                auto&& jsonChannelDes = jsonProbe.get_child( "channel_des" );
                cadProbe.channel_des( jsonChannelDes.root );

                auto&& jsonSequence = jsonProbe.get_child( "sequence" );
                cadProbe.sequence( jsonSequence.root );
                // cadProbe.write();
            }
            // cadProbeset.write();
        }

    }
};

}}}
