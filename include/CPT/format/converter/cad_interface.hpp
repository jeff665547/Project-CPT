#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>

namespace cpt {
namespace format {

namespace cad {
    class ProbesetListView;
}

    class Cad
    {
        Cad () = default;
        void open_cad( const std::string& path );
        void close();

        bool is_probeset_empty ();

        int16_t magic() { return }
        std::string version (){}
        std::string array_type_name (){}
        std::string array_type_version (){}
        uint16_t num_rows ();
        uint16_t num_cols ();
        std::size_t num_probes ();
        std::size_t num_features (){}
        int16_t num_channels (){}
        int max_seq_length (){}
        std::string genome_assembly (){}
        std::string probe_direction (){}
        std::size_t num_probesets ();

        void num_cols ( const uint16_t& i ){}
        void num_rows ( const uint16_t& i ){}
        void magic ( int16_t i ){}
        void version ( const std::string& i ){}
        void array_type_name ( const std::string& i ){}
        void array_type_version ( const std::string& i ){}
        void num_features ( const std::size_t& i ){}
        void num_channels (int16_t i){}
        void max_seq_length (int i){}
        void genome_assembly (const std::string& i){}
        void probe_direction (const std::string& i){}

        cad::ProbesetListView& create_probeset_list( const std::size_t& total_probeset_num, const std::size_t& total_probe_num );
        cad::ProbesetListView& get_probeset_list();

        std::string name    ( const std::size_t& pset_id ){}
        std::string type    ( const std::size_t& pset_id ){}
        std::string subtype ( const std::size_t& pset_id ){}
        std::string chrom   ( const std::size_t& pset_id ){}
        std::size_t start   ( const std::size_t& pset_id ){}
        std::size_t end     ( const std::size_t& pset_id ){}
        std::string desc    ( const std::size_t& pset_id ){}
        char        strand  ( const std::size_t& pset_id ){}

        std::size_t                 shape_name      ( const std::size_t& pset_id, const std::size_t& pn ) {}
        std::size_t                 probe_name      ( const std::size_t& pset_id, const std::size_t& pn ) {}
        std::vector<int>            region_des      ( const std::size_t& pset_id, const std::size_t& pn ) {}
        boost::property_tree::ptree channel_des     ( const std::size_t& pset_id, const std::size_t& pn ) {}
        boost::property_tree::ptree sequence        ( const std::size_t& pset_id, const std::size_t& pn ) {}

    };


} // namespace format
} // namespace cpt
