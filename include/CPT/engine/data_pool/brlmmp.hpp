#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <set>
#include <CPT/logger.hpp>
#include <CPT/algorithm/brlmmp.hpp>
#include <CPT/format/json.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class BRLMMp
{
  public:

    std::vector<                    // pre Probeset
        std::vector<                // pre Cluster
            std::pair<
                  double            // each Cluster Likelihood
                , cpt::algorithm::BRLMMpBufferType  // each Cluster Stuff
            >
        >
    > probeset_likelihood_clusters;

    std::vector<                    // pre Probeset
        std::vector<                // pre Cluster
            std::pair<
                  double            // each Cluster bic
                , size_t            // each Cluster K
            >
        >
    > probeset_bic_clusters;        // sorted

    cpt::algorithm::BRLMMp brlmmp;

    template<class DB>
    BRLMMp( DB& db ) 
    : brlmmp( get_brlmmp( db ))
    {
    }

    template< class DB >
    cpt::algorithm::BRLMMp get_brlmmp( DB& db )
    {
        size_t k_cluster( 3 );
        std::string brlmmp_type( "Gaussian" );

        auto pipeline_schema( cpt::format::make_json( db.pipeline_schema() ));

        for( auto& component : pipeline_schema.get_list( "pipeline" ))
        {
            auto comp = cpt::format::make_json( component.second );
            if( comp.template get_optional<std::string>( "name" ).value_or( "" ) == "Train:BRLMMpTraining" ||
                comp.template get_optional<std::string>( "name" ).value_or( "" ) == "Train:BRLMMpTrainingTentativeClustering" )
            {
                auto parameter = comp.get_child( "parameter" );

                k_cluster   = parameter.template get_optional< size_t    >( "k_cluster.content"   ).value_or( 3 );
                brlmmp_type = parameter.template get_optional<std::string>( "brlmmp_type.content" ).value_or( "Gaussian" );
                break;
            }
        }

        return cpt::algorithm::BRLMMp( k_cluster, brlmmp_type );
    }
};

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
