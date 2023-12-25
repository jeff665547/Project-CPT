#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/components/named_component.hpp>
#include <CPT/format/cube.hpp>
#include <CPT/format/gender/g_chrXY_ratio_caller.hpp>
#include <CPT/format/gender/g_file_loader.hpp>
#include <CPT/format/special_SNP.hpp>
#include <CPT/format/genotype_hints.hpp>
#include <CPT/logger.hpp>
#include <CPT/format/json.hpp>
#include <CPT/engine/data_pool/component_object_manager/icp_obj.hpp>
#include <boost/filesystem.hpp>
#include <mutex>

namespace cpt {
namespace component {
namespace cf = cpt::format;
namespace cfg = cf::gender;
namespace com_ = cpt::engine::data_pool::component_object_manager;
class AuxiliaryParamsLoader : public engine::NamedComponent
{
    using Base = engine::NamedComponent;
    using Tsvs = std::vector< cpt::format::Tsv<> >;
    using cfgbase = cfg::IGender;
    using SpecialSNPs = cpt::format::SpecialSNPsHelper::SpecialSNPsTable;

    /* input */
    com_::ICPObjPtr< std::string >                            genders_file_ipath_;
    com_::ICPObjPtr< std::string >                            chrXprobes_file_ipath_;
    com_::ICPObjPtr< std::string >                            chrYprobes_file_ipath_;
    com_::ICPObjPtr< std::string >                            specialsnps_file_ipath_;
    com_::ICPObjPtr< std::string >                            genotype_hints_file_ipath_;
    
    /* output */
    com_::ICPObjPtr<
        Tsvs 
    >                                                         tsvs_;
    com_::ICPObjPtr<
        cpt::format::Cube< double > 
    >                                                         raw_sample_cube_; 
    com_::ICPObjPtr<
        cpt::format::Cube< double > 
    >                                                         specialsnps_info_cube_;    
    com_::ICPObjPtr<
        std::vector< cfgbase::GenderType > 
    >                                                         genders_;
    com_::ICPObjPtr<
        cpt::format::Cube< double > 
    >                                                         sample_specialsnps_cn_cube_;
    com_::ICPObjPtr<
        cpt::format::Cube< double > 
    >                                                         sample_genohints_cube_;

    std::string                                               genders_caller_method;
    bpt::ptree                                                genders_caller_params;
    std::unique_ptr< cfgbase >                                genders_caller;
    
    bool                                                      is_tsv_;
    std::size_t                                               thread_num_;

  protected:
    template<class T>
    void make_genders_caller_config( const cf::Json<T>& json )
    {
        genders_caller_method = json.template get_optional<std::string> ("sample_genders.content.method").value_or( "" );
        if( genders_caller_method.empty() ) return;
        auto         jgenders = json.         get_child                 ("sample_genders.content.params");
        genders_caller_params = jgenders.root;

        genders_file_ipath_    = com_::require_w< std::string > (
              jgenders
            , "genders_file_path"
            ,  com_::make_ref_parameter("genders_file_path")
        );
        chrXprobes_file_ipath_ = com_::require_w< std::string > (
              jgenders
            , "chrXprobes_file_path"
            ,  com_::make_ref_parameter("chrXprobes_file_path")
        );
        chrYprobes_file_ipath_ = com_::require_w< std::string > (
              jgenders
            , "chrYprobes_file_path"
            ,  com_::make_ref_parameter("chrYprobes_file_path")
        );

        genders_file_ipath_    -> initialize();
        chrXprobes_file_ipath_ -> initialize();                
        chrYprobes_file_ipath_ -> initialize();

        genders_caller_params.put("genders_file_path.val",    genders_file_ipath_    -> get());
        genders_caller_params.put("chrXprobes_file_path.val", chrXprobes_file_ipath_ -> get());
        genders_caller_params.put("chrYprobes_file_path.val", chrYprobes_file_ipath_ -> get());
    }
    template<class T>
    void genders_caller_setup( const cf::Json<T>& json )
    {
        if ( false ) {}
        else if(genders_caller_method == "gender_file_input")
        {
            genders_caller.reset( new cfg::gFileLoader      ( json, raw_sample_cube_ -> get() ) );
        }
        else if(genders_caller_method == "chrXY_ratio")
        {
            genders_caller.reset( new cfg::gChrXYRatioCaller( json, raw_sample_cube_ -> get() ) );
        }
        else
        {
            std::cout << "The method for gender caller is not set or supported ... " << std::endl;
        }
    }
    /* Read json config (pipline.Input:DataLoader & context.*). */
    virtual void config_parameters( const bpt::ptree& p ) override
    {
        auto& db( this->mut_data_pool() );
        /* config thread pool */
        auto json = cf::make_json( p );

        tsvs_                       = com_::require_w< Tsvs > (
              json
            , "tsvs"
            ,  com_::make_ref_parameter("DataLoader")
        );/* "DataLoader" */
        raw_sample_cube_            = com_::require_w< cpt::format::Cube<double> > (
              json
            , "raw_sample_cube"
            , com_::make_ref_parameter("raw_sample_cube")
        );
        genders_                    = com_::require_w< std::vector<cfgbase::GenderType> > (
              json
            , "genders"
            , com_::make_ref_parameter("genders")
        );
        specialsnps_file_ipath_     = com_::require_w< std::string > (
              json
            , "chip_specialSNPs"
            ,  com_::make_ref_parameter("chip_specialSNPs")
        );
        specialsnps_info_cube_      = com_::require_w< cpt::format::Cube<double> > (
              json
            , "specialsnps_info_cube"
            ,  com_::make_ref_parameter("specialsnps_info_cube")
        );        
        sample_specialsnps_cn_cube_ = com_::require_w< cpt::format::Cube<double> > (
              json
            , "sample_special_snps_copynumbers_cube"
            ,  com_::make_ref_parameter("sample_special_snps_copynumbers_cube")
        );
        genotype_hints_file_ipath_  = com_::require_w< std::string > (
              json
            , "sample_genotype_hints"
            ,  com_::make_ref_parameter("sample_genotype_hints")
        );
        sample_genohints_cube_      = com_::require_w< cpt::format::Cube<double> > (
              json
            , "sample_genotype_hints_cube"
            ,  com_::make_ref_parameter("sample_genotype_hints_cube")
        );

        make_genders_caller_config( json );

        is_tsv_     = p.get_optional< bool >( "is_tsv.content" ).value_or( false );
        thread_num_ = p.get_optional< size_t >( "thread_num.content" ).value_or( 16 );
    }
    
  public:

    using Base::Base;
    
    virtual void initialize() override
    {
        auto& db( this->mut_data_pool() );
        tsvs_                       -> initialize();
        specialsnps_file_ipath_     -> initialize();
        genotype_hints_file_ipath_  -> initialize();
        raw_sample_cube_            -> initialize();
        specialsnps_info_cube_      -> initialize();
        genders_                    -> initialize();
        sample_specialsnps_cn_cube_ -> initialize();
        sample_genohints_cube_      -> initialize();
        genders_caller_setup( cf::make_json( genders_caller_params ) );
        db.thread_pool      -> resize_pool( thread_num_ );
    }

    virtual void start() override
    {
        auto& db( this->mut_data_pool() );
        auto& monitor = db.monitor();
        auto& tsvs                       = tsvs_                       -> get();
        auto& specialsnps_file_ipath     = specialsnps_file_ipath_     -> get();
        auto& genotype_hints_file_ipath  = genotype_hints_file_ipath_  -> get();
        auto& raw_sample_cube            = raw_sample_cube_            -> get();
        auto& specialsnps_info_cube      = specialsnps_info_cube_      -> get();
        auto& genders                    = genders_                    -> get();
        auto& sample_specialsnps_cn_cube = sample_specialsnps_cn_cube_ -> get();
        auto& sample_genohints_cube      = sample_genohints_cube_      -> get();

        monitor.set_monitor( "Component Auxiliary Parameters Loader", 5 );        
        
        monitor.log( "Component Auxiliary Parameters Loader", "specialSNPs ... " );

        cpt::format::SpecialSNPsHelper special_snps_helper(specialsnps_file_ipath);
        {
            auto ps_names = special_snps_helper.get_probeset_names();

            cpt::format::Cube<double> cube( ps_names.size(), 2, 1 );
            specialsnps_info_cube.resize  ( ps_names.size(), 2, 1 );

            cube.slice(0) = special_snps_helper.get_copynumber_table();
            cube.x_axis.set_labels( ps_names );
            cube.y_axis.set_labels( std::vector< std::string >({ "Female", "Male" }) );
            cube.z_axis.set_labels( std::vector< std::string >({ "special_snps_info" }) );

            specialsnps_info_cube.swap( cube );
        }


        if( !raw_sample_cube.empty() and !genders_caller_method.empty() )
        {
            monitor.log( "Component Auxiliary Parameters Loader", "genders ... " );
            genders = genders_caller -> get_genders();

            monitor.log( "Component Auxiliary Parameters Loader", "specialSNPs x genders ... " );
            auto ps_names     = special_snps_helper.get_probeset_names();
            auto arma_genders = arma::conv_to<arma::uvec>::from( genders );

            cpt::format::Cube<double>  cube   ( ps_names.size(), genders.size(), 1 );
            sample_specialsnps_cn_cube.resize ( ps_names.size(), genders.size(), 1 );

            cube.slice(0) = special_snps_helper.get_sample_ps_cn_from( arma_genders );
            cube.x_axis.set_labels( ps_names );
            cube.y_axis.set_labels( genders_caller -> get_sample_names() );
            cube.z_axis.set_labels( std::vector< std::string >({ "sample_special_snps_copynumbers" }) );
            
            sample_specialsnps_cn_cube.swap( cube );
        }

        monitor.log( "Component Auxiliary Parameters Loader", "genotype hints ... " );
        cpt::format::GenotypeHintsHelper genotype_hints_helper(genotype_hints_file_ipath);        
        {
            auto ps_names = genotype_hints_helper.get_probeset_names();
            auto sp_names = genotype_hints_helper.get_sample_names();
            auto mat      = genotype_hints_helper.get_genohints_table();

            cpt::format::Cube<double> cube ( mat.n_rows, mat.n_cols, 1 );
            sample_genohints_cube.resize   ( mat.n_rows, mat.n_cols, 1 );
        
            cube.slice(0) = mat;
            cube.x_axis.set_labels( ps_names );
            cube.y_axis.set_labels( sp_names );
            cube.z_axis.set_labels( std::vector< std::string >({ "sample_genotype_hints" }) );
            
            sample_genohints_cube.swap( cube );
        }

        monitor.log( "Component Auxiliary Parameters Loader", "Complete!!!" );
    }

};

} // end of namespace component
} // end of namespace cpt
#else
#endif
