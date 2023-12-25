#pragma once
#include <boost/filesystem.hpp>
#include <CPT/format/json.hpp>
#include <CPT/format/cube.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

namespace cpt {
namespace format {
struct GenotypeHintsHelper {
    arma::mat                                      genohints;
    std::vector<std::string>                       probeset_names;
    std::vector<std::string>                       sample_names;

    GenotypeHintsHelper( const std::string& genotype_hints_path )
    {
        require_genohints( genotype_hints_path );
        if( !genotype_hints_path.empty() and genohints.empty() )
            throw std::runtime_error("Genotype hints file loading error. Please check its file path, json settings, and format!");
    }

    bool valid( const arma::imat& tmpmat ) const
    {
        return ( tmpmat.n_rows == probeset_names.size() ) and
               ( tmpmat.n_cols == sample_names  .size() ) and
               ( tmpmat.n_rows > 0 ) and ( tmpmat.n_cols > 0 );
    }

    void load_genohints( const std::string& genotype_hints_path )
    {
        // def: 0 <=> AA, 1 <=> AB, 2 <=> BB, -1 <=> No call (default)
        genohints     .clear();
        probeset_names.clear();
        sample_names  .clear();

        std::string                  tmp_str;
        std::ifstream                ifs( genotype_hints_path );
        boost::archive::xml_iarchive ar(ifs);
        ar & boost::serialization::make_nvp("Genohints_table",         tmp_str          );
        ar & boost::serialization::make_nvp("Probeset_names_for_row",  probeset_names  );
        ar & boost::serialization::make_nvp("Sample_names_for_col",    sample_names    );
        if(!tmp_str.empty() && tmp_str[0] == '\n') {
            tmp_str.erase(0, 1);
        }
        std::stringstream ss( tmp_str );
        {
            arma::imat tmpmat;
            tmpmat.load(ss);
            if( !valid(tmpmat) )
            {
                throw std::runtime_error("Error opening genohints file! Format Error\n");
            }
            genohints = arma::conv_to<arma::mat>::from( tmpmat );
        }
    }

    void require_genohints( const std::string& genotype_hints_path )
    {
        if( !genotype_hints_path.empty() )
        {
            if( boost::filesystem::exists( genotype_hints_path ) )
            {
                cpt::verbose0 << "require genohints from: " << genotype_hints_path << std::endl;
                load_genohints( genotype_hints_path );
            }
        }
    }

    auto get_probeset_names() const
    {
        return probeset_names;
    }

    auto get_sample_names() const
    {
        return sample_names;
    }

    auto get_genohints_table() const
    {
        return genohints;
    }
};
} // end of namespace format
} // end of namespace cpt
