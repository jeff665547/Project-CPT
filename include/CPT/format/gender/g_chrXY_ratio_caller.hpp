#pragma once
#include <cstdint>
#include <CPT/format/gender/igender.hpp>
#include <CPT/format/json.hpp>
#include <CPT/format/cube.hpp>
#include <CPT/format/chr_sex.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <utility>

namespace cpt {
namespace format {
namespace gender {

struct gChrXYRatioCaller : public IGender {
    
    cpt::format::ChrSex      ChrX;
    cpt::format::ChrSex      ChrY;
    double                   female_threshold;
    double                   male_threshold;

    template<class T>
    gChrXYRatioCaller( const cpt::format::Json<T>& json, const cpt::format::Cube<double>& cube ) 
    : ChrX              ( json.template get<std::string>      ("chrXprobes_file_path.val")                )
    , ChrY              ( json.template get<std::string>      ("chrYprobes_file_path.val")                )
    , female_threshold  ( json.template get_optional<double>  ("female_threshold")        .value_or(0.54) )
    , male_threshold    ( json.template get_optional<double>  ("male_threshold")          .value_or(1.0)  )
    , expression_cube   ( cube )
    {
        if( ChrX.probe_id_channel_pairs.empty() or ChrY.probe_id_channel_pairs.empty() )
            throw std::runtime_error("chrXprobes or chrYprobes file loading error. "
                                     "Please check their file paths, json settings, and format!");
    }

    void require_chrXY( const std::string& chrX_file_path, const std::string& chrY_file_path )
    {   
        ChrX.require_chr_sex(chrX_file_path);
        ChrY.require_chr_sex(chrY_file_path);
    }
    void save_chrXY( const std::string& chrX_file_path, const std::string& chrY_file_path )
    {
        ChrX.save_chr_sex(chrX_file_path);
        ChrY.save_chr_sex(chrY_file_path);
    }
    std::vector<std::string> get_sample_names() const override
    {
        return expression_cube.y_axis.get_labels();
    }
    std::vector<GenderType> get_genders( const arma::mat& chrX_intensities, const arma::mat& chrY_intensities ) const
    {
        // Row: probesets, Col: samples
        arma::rowvec mean_chrX   = arma::mean(chrX_intensities, 0);
        arma::rowvec mean_chrY   = arma::mean(chrY_intensities, 0);

        arma::rowvec chrXY_ratio = mean_chrY / mean_chrX;

        arma::uvec    male_id    = arma::find(chrXY_ratio >  male_threshold );
        arma::uvec   female_id   = arma::find(chrXY_ratio < female_threshold);

        // Don't know genotype, use XX (female) model as default.
        std::vector<GenderType> genders{ chrXY_ratio.n_elem, Female };
        for(const auto& id: male_id) 
        {
            genders[id] = Male;
        }
        for(const auto& id: female_id) 
        {
            genders[id] = Female;
        }

        return genders;
    }
    std::vector<GenderType> get_genders() const override
    {
        arma::mat chrX_intensities (ChrX.probe_id_channel_pairs.size(), expression_cube.n_cols, arma::fill::zeros);
        arma::mat chrY_intensities (ChrY.probe_id_channel_pairs.size(), expression_cube.n_cols, arma::fill::zeros);
        int row_id = 0;
        for(const auto& [pb_id, ch_id]: ChrX.probe_id_channel_pairs)
        {
            chrX_intensities.row(row_id) = expression_cube.slice(ch_id.val).row(pb_id.val);
            row_id++;
        }

        row_id = 0;
        for(const auto& [pb_id, ch_id]: ChrY.probe_id_channel_pairs)
        {
            chrY_intensities.row(row_id) = expression_cube.slice(ch_id.val).row(pb_id.val);
            row_id++;
        }

        return get_genders(chrX_intensities, chrY_intensities);
    }
  private:
    const cpt::format::Cube<double>& expression_cube;
};

} // end of namespace gender
} // end of namespace format
} // end of namespace cpt
