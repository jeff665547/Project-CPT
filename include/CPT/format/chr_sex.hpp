#pragma once
#include <cstdint>
#include <CPT/format/gender/igender.hpp>
#include <CPT/format/json.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

namespace cpt {
namespace format {

struct ChrSex {
public:
    ChrSex( const std::string& chr_sex_file_path )
    {
        require_chr_sex(chr_sex_file_path);
    }
    void load_chr_sex(const std::string& chr_sex_file_path)
    {
        probe_id_channel_pairs.clear();
        cpt::verbose0 << "load chr sex info from: " << chr_sex_file_path << std::endl;
        std::string line;
        std::ifstream ifs(chr_sex_file_path);
        bool header_check = false;
        if(!ifs.is_open())
        {
            throw std::runtime_error("Error opening chrsexprobes file!\n");
        }
        while( std::getline(ifs, line) )
        {
            if( line.at( 0 ) == '#' )
            {
                continue;
            }

            if( (line == "probe_id\tchannel") || (line == "probe_id\tchannel_id") )
            {
                header_check = true;
                continue;
            }
            
            if(header_check)
            {
                probe_id     pid;
                channel_id  chid;
                std::istringstream iss(line);
                if( !(iss >> pid.val >> chid.val) )
                {
                    throw std::runtime_error("Error parsing line in chrsexprobes file: " + line + "\n");
                }
                pid.val--;
                probe_id_channel_pairs.emplace_back(pid, chid);
            }
        }

        ifs.close();
    }
    void save_chr_sex(const std::string& chr_sex_file_path)
    {
        cpt::verbose0 << "save chr sex info to: " << chr_sex_file_path << std::endl;
        std::ofstream ofs(chr_sex_file_path);
        if(!ofs.is_open())
        {
            throw std::runtime_error("Error opening chrsexprobes file!\n");
        }
        ofs << "probe_id\tchannel_id" << std::endl;
        for(const auto& [p_id, ch_id]: probe_id_channel_pairs)
        {
            ofs << p_id.val+1 << "\t" << ch_id.val << std::endl;
        }
        ofs.close();
    }
    void require_chr_sex(const std::string& chr_sex_file_path)
    {
        if(!chr_sex_file_path.empty()) 
        {
            if(boost::filesystem::exists(chr_sex_file_path))
            {
                cpt::verbose0 << "require chr sex info from: " << chr_sex_file_path << std::endl;
                load_chr_sex(chr_sex_file_path);
            }
        }
    }
    auto get_probes_channel_info() const
    {
        return probe_id_channel_pairs;
    }
    struct probe_id
    {
        uint32_t val;
    };
    struct channel_id
    {
        std::size_t val;
    };

    std::vector<std::pair<probe_id, channel_id>> probe_id_channel_pairs;
};

} // end of namespace format
} // end of namespace cpt
