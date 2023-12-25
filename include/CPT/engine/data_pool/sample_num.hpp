#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/format/celfile.hpp>
#include <CPT/format/cube.hpp>
#include <set>

namespace cpt {
namespace engine {
namespace data_pool {

class SampleNum
{

  public:

    // CELFile cel;

    size_t num_samples;

    template< class DB >
    SampleNum( DB& db ) 
        : num_samples( 0 )
    {
        #ifdef NEW_DATA_POOL
        auto& pipeline_schema_input( db.pipeline_schema().get_child( "context" ));
        for( auto& child : pipeline_schema_input.get_child( "sample_files" ))
        {
            num_samples++;
            // db.push_path( "sample_files", child.second );
        }
        #else
        auto& pipeline_schema_input( db.pipeline_schema().get_child( "input" ));
        for( auto& child : pipeline_schema_input.get_child( "sample_files" ))
        {
            num_samples++;
            db.push_path( "sample_files", child.second );
        }
        #endif 

    }
// 
//     template< class DB >
//     void load_sample_cel( const std::string& fname, const size_t& sample, DB& db )
//     {
//         auto cel = CELFile::load( fname );
// 
//         for( size_t channel = 0; channel != 2; ++channel )
//         {
//             auto probe_vec = cel->extract_intensities( channel );
// 
//             for( size_t idx = 0; idx < probe_vec.size(); ++idx )
//             {
//                 db.raw_sample_cube( idx, sample, channel ) = probe_vec[ idx ];
//             }
// 
//             // for( auto& probeset : db.probeset_table )
//             // {
//             //     for( size_t idx = 0; idx < probeset.probes.size(); ++idx )
//             //     {
//             //         if( probeset.channels[ idx ] == channel )
//             //         {
//             //             db.raw_sample_cube( probeset.probes[ idx ], sample, channel ) = probe_vec[ probeset.probes[ idx ]];
//             //         }
//             //     }
//             // }
//         }
//     }
// 
//     template< class DB >
//     void require_sample_cel( const std::string& fname, const size_t& sample_num, DB& db )
//     {
//         if( is_cel_loaded_.find( fname ) == is_cel_loaded_.end() )
//         {
//             load_sample_cel( fname, sample_num, db );
//             is_cel_loaded_.emplace( fname );
//         }
//     }
};

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
