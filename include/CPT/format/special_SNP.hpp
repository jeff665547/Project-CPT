#pragma once
#include <boost/filesystem.hpp>
#include <CPT/format/json.hpp>
#include <CPT/format/cube.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

namespace cpt {
namespace format {
struct SpecialSNPsHelper {
    struct CopynumberInfo
    {
        std::string     region;
        unsigned short  copy_male;
        unsigned short  copy_female;

        CopynumberInfo()
        : copy_male    (2)
        , copy_female  (2)
        , region       ("AUTO")
        {}
    };

    using SpecialSNPsTable = std::vector<
        std::pair< std::string, CopynumberInfo >
    >;

    SpecialSNPsTable           probeset2gender_copy_vec;
    std::vector<std::string>   SNP_names;
    std::vector<std::string>   regions;
    arma::mat                  cn_val_table;

    SpecialSNPsHelper() = default;
    SpecialSNPsHelper( const std::string& specialSNPs_path ) 
    {
        require_specialSNPs( specialSNPs_path );
        if( !specialSNPs_path.empty() and cn_val_table.empty() )
            throw std::runtime_error("specialSNPs file loading error. Please check its file path, json settings, and format!");
    }
    void load_specialSNPs(const std::string& specialSNPs_path)
    {
        probeset2gender_copy_vec.clear();
        SNP_names               .clear();
        regions                 .clear();
        cpt::verbose0 << "load specialSNPs from: " << specialSNPs_path << std::endl;
        std::string line;
        std::ifstream ifs(specialSNPs_path);
        std::stringstream cn_tmp;
        bool header_check = false;
        if(!ifs.is_open())
        {
            throw std::runtime_error("Error opening specialSNPs file!\n");
        }
        while( std::getline(ifs, line) )
        {
            if( line.at( 0 ) == '#' )
            {
                continue;
            }

            if( (line == "probeset_id\tchr\tcopy_male\tcopy_female") )
            {
                header_check = true;
                continue;
            }
            
            if(header_check)
            {
                std::istringstream iss(line);
                std::string        SNP_name;
                CopynumberInfo     cn_info;
                if( !( iss >> SNP_name >> cn_info.region >> cn_info.copy_male >> cn_info.copy_female ) )
                {
                    throw std::runtime_error("Error parsing line in specialSNPs file: " + line + "\n");
                }
                SNP_names               .emplace_back( SNP_name );
                regions                 .emplace_back( cn_info.region );
                probeset2gender_copy_vec.emplace_back( SNP_name, cn_info );
                cn_tmp << cn_info.copy_female << " " << cn_info.copy_male << std::endl;
            }
        }
        ifs.close();
        cn_val_table.load(cn_tmp);
    }
    void save_specialSNPs(const std::string& specialSNPs_path)
    {
        cpt::verbose0 << "save specialSNPs to: " << specialSNPs_path << std::endl;
        std::ofstream ofs(specialSNPs_path);
        if(!ofs.is_open())
        {
            throw std::runtime_error("Error opening specialSNPs file!\n");
        }
        ofs << "probeset_id\tchr\tcopy_male\tcopy_female" << std::endl;
        for(const auto& [pb, cn_info]: probeset2gender_copy_vec)
        {
            ofs << pb << "\t" << cn_info.region << "\t" << cn_info.copy_male << "\t" << cn_info.copy_female << std::endl;
        }
        ofs.close();
    }
    void require_specialSNPs(const std::string& specialSNPs_path)
    {
        if(!specialSNPs_path.empty()) 
        {
            if(boost::filesystem::exists(specialSNPs_path))
            {
                cpt::verbose0 << "require specialSNPs from: " << specialSNPs_path << std::endl;
                load_specialSNPs(specialSNPs_path);
            }
        }
    }
    auto get_mapping() const
    {
        return probeset2gender_copy_vec;
    }
    auto get_probeset_names() const
    {
        return SNP_names;
    }
    auto get_regions() const
    {
        return regions;
    }
    auto get_copynumber_table() const
    {
        return cn_val_table;
    }
    CopynumberInfo get_copynumber_info( const std::string& SNP_name ) const
    {
        auto it = std::find_if(probeset2gender_copy_vec.begin(), probeset2gender_copy_vec.end(),
                               [&SNP_name](const auto& row_SNP_data) { return row_SNP_data.first == SNP_name; });
        if(it != probeset2gender_copy_vec.end()) {
            return it->second;
        }
        return {};
    }
    auto get_gender_copynumber( const std::string& SNP_name ) const
    {
        CopynumberInfo res_copynumber_info = get_copynumber_info( SNP_name );
        return std::make_tuple( res_copynumber_info.copy_female, res_copynumber_info.copy_male );
    }
    arma::mat get_sample_ps_cn_from( const arma::uvec& sample_genders ) const
    {
        return cn_val_table.cols( sample_genders );
    }
};

} // end of namespace format
} // end of namespace cpt
