#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/engine/data_pool/monitor.hpp>
#include <CPT/format/cdffile.hpp>
#include <CPT/logger.hpp>
#include <algorithm>
#include <map>

namespace cpt {
namespace engine {
namespace data_pool {

class CdfImpl
{
  protected:

    bool is_cdf_loaded_;

  public:

    cpt::format::CDFFile cdf;

  private:

    Monitor< decltype( cpt::msg )>& monitor_;

  public:

    template< class DB >
    CdfImpl( DB& db ) 
        : is_cdf_loaded_( false )
        , monitor_( db.monitor() )
    {
        #ifdef NEW_DATA_POOL
        auto& pipeline_schema_input( db.pipeline_schema().get_child( "context" ));
        #else
        auto& pipeline_schema_input( db.pipeline_schema().get_child( "input" ));

        for( auto& child : pipeline_schema_input.get_child( "chip_layout" ))
        {
            db.push_path( "chip_layout", child.second );
        }
        #endif
    }

    void load_cdf( const std::string& fname )
    {
        std::ifstream f( fname, std::ios::binary );

        // cpt::logout1 << ": " << fname << "\n";

        cdf.open( f );
        f.close();
    }

    void require_cdf( const std::string& fname )
    {
        if( !is_cdf_loaded_ )
        {
            monitor_.set_monitor( "CDF File", 2 );
            monitor_.log( "CDF File", "Loading: " + fname );

            // cpt::logout1 << "\tLoad CDF File";

            load_cdf ( fname );
            is_cdf_loaded_ = true;

            monitor_.log( "CDF File", "Loading ... Complete" );

            // cpt::logout1 << "\r\tLoad CDF File Complete\n";
        }
    }
};
using Cdf = CdfImpl;

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
