#pragma once
#include <CPT/format/gender/igender.hpp>
#include <CPT/format/json.hpp>
#include <CPT/format/cube.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

namespace cpt {
namespace format {
namespace gender {

struct gFileLoader : public IGender {
public:
    template<class T>
    gFileLoader( const cpt::format::Json<T>& json, const cpt::format::Cube<double>& cube ) 
    : expression_cube( cube )
    {
        require_gender( json.template get<std::string>("genders_file_path.val") );
        if( filename2gendertype_map.empty() )
            throw std::runtime_error("gender file loading error. Please check its file path, json settings, and format!");
    }
    void load_gender( const std::string& gender_path )
    {
        filename2gendertype_map.clear();
        input_filenames.clear();
        cpt::verbose0 << "load gender from: " << gender_path << std::endl;
        std::string line;
        std::ifstream ifs(gender_path);
        bool header_check = false;
        if(!ifs.is_open())
        {
            throw std::runtime_error("Error opening gender file!\n");
        }
        while( std::getline(ifs, line) )
        {
            if( line.at( 0 ) == '#' )
            {
                continue;
            }

            if( (line == "gender\tsample_files") )
            {
                header_check = true;
                continue;
            }
            
            if(header_check)
            {
                std::istringstream iss(line);
                std::string        gender_str;
                std::string        sample_file_name;
                GenderType         sample_gender;
                if( !(iss >> gender_str >> sample_file_name) || !valid(gender_str, sample_gender) )
                {
                    throw std::runtime_error("Error parsing line in genders file: " + line + "\n");
                }
                filename2gendertype_map[sample_file_name] = sample_gender;
                input_filenames.emplace_back(sample_file_name);
            }
        }
        ifs.close();
        if( !input_filenames.empty() )
        {
            boost::filesystem::path p ( input_filenames[0] );
            filetype = p.extension().string();
        }
    }
    void save_gender( const std::string& gender_path )
    {
        cpt::verbose0 << "save genders to: " << gender_path << std::endl;
        std::ofstream ofs(gender_path);
        if(!ofs.is_open())
        {
            throw std::runtime_error("Error opening gender file!\n");
        }
        ofs << "gender\tsample_files" << std::endl;
        for(const auto& [sample_file_name, gender]: filename2gendertype_map)
        {
            ofs << gender << "\t" << sample_file_name << std::endl;
        }
        ofs.close();
    }
    void require_gender( const std::string& gender_path )
    {
        if(!gender_path.empty()) 
        {
            if(boost::filesystem::exists(gender_path))
            {
                cpt::verbose0 << "require gender from: " << gender_path << std::endl;
                load_gender(gender_path);
            }
        }
    }
    std::vector<std::string> get_sample_names() const override
    {
        return expression_cube.y_axis.get_labels();
    }
    GenderType get_gender( const std::string& sample_file_name ) const
    {
        auto it = filename2gendertype_map.find(sample_file_name);
        if(it != filename2gendertype_map.end()) {
            return it->second;
        }
        return Female;   // Don't know genotype, use XX (female) model as default.
    }
    std::vector<GenderType> get_genders() const override
    {
        const auto& sample_names = get_sample_names();
        std::vector<GenderType> genders { sample_names.size(), Female };
        for(auto id = 0; id < sample_names.size(); ++id)
        {
            genders[id] = get_gender( sample_names[id] + filetype );
        }
        return genders;
    }
    std::vector<std::string> get_input_sample_names() const
    {
        std::vector<std::string> sample_names = input_filenames;
        std::for_each(sample_names.begin(), sample_names.end(),
            [ this ](std::string& sample_name) 
            {
                size_t pos = sample_name.find( filetype );
                if(pos != std::string::npos) sample_name.erase(pos, filetype.size());
            }
        );
        return sample_names;
    }
private:
    std::unordered_map<std::string, GenderType> filename2gendertype_map;
    std::vector<std::string> input_filenames;
    const cpt::format::Cube<double>& expression_cube;
    std::string filetype;
};

} // end of namespace gender
} // end of namespace format
} // end of namespace cpt
