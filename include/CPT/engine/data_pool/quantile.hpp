#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/engine/data_pool/monitor.hpp>
#include <CPT/logger.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class Quantile
{
  protected:

    bool is_sketch_loaded_;
    bool is_scaling_loaded_;

  public:

    std::vector< double > sketch_table;
    size_t scaling_factor;

  private:

    Monitor< decltype( cpt::msg )>& monitor_;

  public:

    template< class DB >
    Quantile( DB& db )
        : is_sketch_loaded_( false )
        , is_scaling_loaded_( false )
        , sketch_table()
        , scaling_factor( 0 )
        , monitor_( db.monitor() )
    {}

    void read_sketch( const std::string& sketch )
    {
        bool header_check = false;
        std::string line;
        std::fstream file( sketch );

        while( std::getline( file, line ) )
        {
            if( line.at( 0 ) == '#' )
            {
                /*
                 *  Other header stuff here
                 */
                continue;
            }

            if( header_check )
            {
                sketch_table.push_back( std::stod( line ));
            }

            if( line == "intensities" )
            {
                header_check = true;
            }
        }

        file.close();
    }

    void load_sketch( const std::string& sketch )
    {
        if( !bfs::exists( sketch ))
        {
            throw std::runtime_error( sketch + " not found" );
        }

        read_sketch( sketch );
        is_sketch_loaded_ = true;
    }

    void require_sketch( const std::string& sketch )
    {
        if( !is_sketch_loaded_ )
        {
            monitor_.set_monitor( "Sketch File", 2 );
            monitor_.log( "Sketch File", "Loading: " + sketch );

            load_sketch( sketch );

            if( sketch_table.empty() )
            {
                cpt::logout1 << ", But The File Is Empty";
                is_sketch_loaded_ = false;
            }

            monitor_.log( "Sketch File", "Loading ... Complete" );
        }
    }

    void require_scaling_factor( const size_t& scaling )
    {
        if( !is_scaling_loaded_ )
        {
            scaling_factor = scaling;
            is_scaling_loaded_ = true;
        }
    }

     void write_sketch( const std::string& sketch )
     {
         monitor_.set_monitor( "Sketch File", sketch_table.size() + 1 );

         std::ofstream output( sketch );

         /*
          *  Other header stuff here
          */

         output << "intensities\n";

         for( auto& sk : sketch_table )
         {
             monitor_.log( "Sketch File", "Writing: " + sketch );

             output << std::to_string( sk ) << "\n";
         }

         output.close();

         monitor_.log( "Sketch File", "Wtriting ... Complete" );
     }


    void save_sketch( const std::string& sketch )
    {
        write_sketch( sketch );
    }

    void clear_sketch()
    {
        if( is_sketch_loaded_ )
        {
            std::vector< double >().swap( sketch_table );
            is_sketch_loaded_ = false;
        }
    }
};

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
