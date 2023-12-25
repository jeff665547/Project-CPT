#pragma once
#ifdef NEW_DATA_POOL
#include <boost/serialization/serialization.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <CPT/components/axiomGT/probeset_models.hpp>
#include <CPT/format/special_SNP.hpp>
#include <CPT/format/gender/igender.hpp>
#include <unordered_set>

namespace boost {
namespace serialization {
namespace ccagt = cpt::component::axiomGT;

void serialize( boost::archive::xml_oarchive& ar, 
                std::pair<const int, ccagt::probeset_distribution>& info, 
                unsigned int version ) 
{
    // std::cout << "model level output" << std::endl;
    ar & boost::serialization::make_nvp("Copynumber",            info.first  );
    ar & boost::serialization::make_nvp("ModelInfo",             info.second );
}

void serialize( boost::archive::xml_oarchive& ar, 
                std::map<int, ccagt::probeset_distribution>& probeset_info, 
                unsigned int version )
{
    // std::cout << "probeset level output" << std::endl;
    size_t size = probeset_info.size();
    ar & boost::serialization::make_nvp("ProbesetInfoCounts",    size        );
    for(auto& info: probeset_info)
    {
        ar & boost::serialization::make_nvp("Info",              info        );
    }
}

void serialize( boost::archive::xml_iarchive& ar, 
                std::pair<int, ccagt::probeset_distribution>& info, 
                unsigned int version ) 
{
    // std::cout << "model level input" << std::endl;
    ar & boost::serialization::make_nvp("Copynumber",            info.first  );
    ar & boost::serialization::make_nvp("ModelInfo",             info.second );
}

void serialize( boost::archive::xml_iarchive& ar, 
                std::map< int, ccagt::probeset_distribution >& probeset_info, 
                unsigned int version ) 
{
    // std::cout << "probeset level input" << std::endl;
    size_t size;
    ar & boost::serialization::make_nvp("ProbesetInfoCounts",    size        );
    while(size--)
    {
        std::pair<int, ccagt::probeset_distribution> info;
        ar & boost::serialization::make_nvp("Info",              info        );
        probeset_info.insert(info);
    }
}

} // end of serialization
} // end of boost

namespace cpt {
namespace engine {
namespace data_pool {
namespace ccagt = cpt::component::axiomGT;
class AxiomGTAuxiliary
{
    using SpecialSNPsTable = cpt::format::SpecialSNPsHelper::SpecialSNPsTable;
  protected:
    bool loaded_;

  public:
    std::vector< std::map< int, ccagt::probeset_distribution > >        NIW_priors                                  ;
    std::vector< std::map< int, ccagt::probeset_distribution > >        NIW_posteriors                              ;
    std::map< int, ccagt::probeset_distribution >                       NIW_generic_prior                           ;
    arma::imat                                                          genohints                                   ;
    arma::imat                                                          copynumbers                                 ;
    std::vector< std::set< int > >                                      copynumber_categories                       ;
  
  private:
    std::vector<std::string>                                            iprobeset_names                             ;
    std::vector<std::string>                                            isample_names                               ;
    cpt::format::Cube< double >                                         input_copynumber_categories                 ;

  public:
    template<typename T>
    auto idmapping( const std::vector<T>& source, const std::vector<T>& target )
    {
        std::vector<std::size_t> source_id_on_target_order;
        for(const auto& element: target) 
        {
            auto it = std::find(source.begin(), source.end(), element);
            if(it != source.end()) 
            {
                source_id_on_target_order.emplace_back( std::distance( source.begin(), it ) );
            } 
            else 
            {
                source_id_on_target_order.emplace_back( static_cast<std::size_t>( 0 ) );
            }
        }
        return source_id_on_target_order;
    }

    void add_buffer_from_head_for_vec(
        std::vector<std::string>&       new_vec,
        const std::vector<std::string>& old_vec
    )
    {
        new_vec.resize(1, "BUFFER");
        new_vec.insert(
            new_vec.end(),
            old_vec.begin(),
            old_vec.end()
        );
    }

    template< typename TYPE, typename BUFFERTYPE >
    auto add_buffer( 
        const arma::Mat<TYPE>&            input_mat, 
        const std::vector<std::string>&   input_x_labels, 
        const std::vector<std::string>&   input_y_labels,
        const BUFFERTYPE&                 buffer_value
    )
    {
        arma::Mat<TYPE> buffer_mat = buffer_value * arma::ones<arma::Mat<TYPE>>(input_mat.n_rows + 1, input_mat.n_cols + 1);
        buffer_mat.submat( 1, 1,   input_mat.n_rows, input_mat.n_cols ) = input_mat;

        std::vector<std::string> buffer_mat_x_labels;
        add_buffer_from_head_for_vec( buffer_mat_x_labels, input_x_labels );

        std::vector<std::string> buffer_mat_y_labels;
        add_buffer_from_head_for_vec( buffer_mat_y_labels, input_y_labels );

        return std::make_tuple( buffer_mat, buffer_mat_x_labels, buffer_mat_y_labels );
    }
    
    template< typename TYPE, typename IMPUTETYPE >
    auto join_2D_data_by_label(
        const cpt::format::Cube< TYPE >&     input_data_frame,
        const std::vector<std::string>&      inner_Xaxis_names,
        const std::vector<std::string>&      inner_Yaxis_names,
        const IMPUTETYPE&                    impute_value
    )
    {
        auto [ tmp_mat, tmp_probeset_names, tmp_sample_names ] = add_buffer(
            arma::conv_to<arma::Mat<IMPUTETYPE>>::from( input_data_frame.slice(0) ),
            input_data_frame.x_axis.get_labels(),
            input_data_frame.y_axis.get_labels(),
            impute_value
        );
        
        const auto& input2cube_order_row      = idmapping( tmp_probeset_names, inner_Xaxis_names );
        const auto& input2cube_order_col      = idmapping( tmp_sample_names,   inner_Yaxis_names );

        arma::uvec  row_selected_ids          = arma::conv_to<arma::uvec>::from( input2cube_order_row );
        arma::uvec  col_selected_ids          = arma::conv_to<arma::uvec>::from( input2cube_order_col );

        arma::Mat<IMPUTETYPE> row_picked_mat  = tmp_mat       .rows( row_selected_ids );
        arma::Mat<IMPUTETYPE> joined_mat      = row_picked_mat.cols( col_selected_ids );

        return joined_mat;
    }

    template< typename TYPE >
    void set_copynumber_categories_for_each_probeset(
        const cpt::format::Cube< TYPE >&     input_specialsnps_info_cube, 
        const std::vector<std::string>&      expression_cube_probeset_names 
    )
    {
        int autosomal_copynumber = 2;
        copynumber_categories.clear();
        copynumber_categories.resize(
            expression_cube_probeset_names.size(),
            { autosomal_copynumber }
        );
        if( !input_specialsnps_info_cube.empty() )
        {
            cpt::format::Cube< TYPE > tmp( input_specialsnps_info_cube );
            input_copynumber_categories.resize( 
                tmp.n_rows,
                tmp.n_cols,
                tmp.n_slices
             );
            input_copynumber_categories.swap( tmp );
            const auto& content  = arma::conv_to<arma::imat>::from( input_specialsnps_info_cube.slice(0) );
            arma::imat tmp_mat = autosomal_copynumber * arma::ones<arma::imat>( content.n_rows + 1, 2 );
            tmp_mat.submat( 1, 0,   content.n_rows, 1 ) = content;

            std::vector<std::string> tmp_specialsnps_labels;
            add_buffer_from_head_for_vec( 
                tmp_specialsnps_labels, 
                input_specialsnps_info_cube.x_axis.get_labels()
            );
            const auto& input2cube_order_row = idmapping( tmp_specialsnps_labels, expression_cube_probeset_names );
            arma::uvec  row_selected_ids     = arma::conv_to<arma::uvec>::from( input2cube_order_row );
            arma::imat  ps_copynumbers_info  = tmp_mat.rows( row_selected_ids );
            
            copynumber_categories.clear();
            copynumber_categories.resize( expression_cube_probeset_names.size(), {} );
            for( size_t r = 0; r < expression_cube_probeset_names.size(); r++ )
            {
                for( size_t c = 0; c < ps_copynumbers_info.n_cols; c++ )
                {
                    copynumber_categories[r].insert( ps_copynumbers_info(r, c) );
                }
            }
        }
    }

    template< typename TYPE >
    void set_copynumbers_from_sample_special_snps( 
        const cpt::format::Cube< TYPE >&     input_sample_specialsnps_copynumber_cube,
        const std::vector<std::string>&      expression_cube_probeset_names,
        const std::vector<std::string>&      expression_cube_sample_names
    )
    {
        // Default: Fully autosomal SNPs
        int autosomal_copynumber = 2;
        copynumbers = autosomal_copynumber * arma::ones<arma::imat>(
            expression_cube_probeset_names.size(), 
            expression_cube_sample_names.size()
        );
        if( !input_sample_specialsnps_copynumber_cube.empty() )
        {
            copynumbers = join_2D_data_by_label( 
                input_sample_specialsnps_copynumber_cube,
                expression_cube_probeset_names,                
                expression_cube_sample_names,
                static_cast<long long>( autosomal_copynumber )
            );
        }
        else if( !input_copynumber_categories.empty() )
        {
            // unknown genders -> default Female.
            std::vector genders( expression_cube_sample_names.size(), cpt::format::gender::IGender::GenderType::Female );
            auto arma_genders = arma::conv_to<arma::uvec>::from( genders );
            cpt::format::Cube< TYPE > cube( 
                input_copynumber_categories.x_axis.get_labels().size(), 
                expression_cube_sample_names.size(), 
                1 
            );
            auto tmp_mat = input_copynumber_categories.slice(0);
            cube.slice(0) = tmp_mat.cols( arma_genders );
            cube.x_axis.set_labels( input_copynumber_categories.x_axis.get_labels() );
            cube.y_axis.set_labels( expression_cube_sample_names   );

            set_copynumbers_from_sample_special_snps( cube, expression_cube_probeset_names, expression_cube_sample_names );
        }
    }

    template< typename TYPE >
    int set_genohints( 
        const cpt::format::Cube< TYPE >&     input_genohints_cube,
        const std::vector<std::string>&      expression_cube_probeset_names,
        const std::vector<std::string>&      expression_cube_sample_names
    )
    {
        // def: 0 <=> AA, 1 <=> AB, 2 <=> BB, -1 <=> No call
        // Default: unknown genotype (no call)
        int default_genotype = -1;
        genohints = default_genotype * arma::ones<arma::imat>(
            expression_cube_probeset_names.size(), 
            expression_cube_sample_names.size()
        );
        if( !input_genohints_cube.empty() )
        {
            genohints = join_2D_data_by_label( 
                input_genohints_cube,
                expression_cube_probeset_names,                
                expression_cube_sample_names,
                static_cast<long long>( default_genotype )
            );
            return 1;
        }
        return 0;
    }

    template< typename TYPE >
    void initialize_by_copynumber_categories(
        std::vector<std::map<int, TYPE>>& models,
        const std::map<int, TYPE>&        ref_mdl
    )
    {
        assert( !copynumber_categories.empty() );

        models.clear();
        models.resize( copynumber_categories.size() );
        for( size_t ps_id = 0; ps_id != copynumber_categories.size(); ps_id++ )
        {
            for(auto& spec_cn: copynumber_categories[ps_id])
            {
                auto it = ref_mdl.find( spec_cn );
                if( it == ref_mdl.end() )
                    if( spec_cn == 0 ) continue;
                else
                    throw std::runtime_error("Cannot find the model for the copynumber: "  
                                              + std::to_string( spec_cn ) + ".\n");
                models[ps_id].insert({ spec_cn, it->second });
            }
        }
    }

    void read_NIW_priors( const std::string& ipath, const std::vector<std::string>& cube_probeset_names )
    {
        iprobeset_names.clear();
        const std::size_t& probesets_num = cube_probeset_names.size();
        std::vector< std::map< int, ccagt::probeset_distribution > > ipriors;
        std::ifstream ifs ( ipath );
        boost::archive::xml_iarchive ar ( ifs );
        ar & boost::serialization::make_nvp("NIW_models",     ipriors       );
        for(const auto& ps_iprior: ipriors)
        {
            auto it = ps_iprior.begin();
            iprobeset_names.emplace_back( it->second.name );
        }
        if( iprobeset_names[0] != "GENERIC" )
            throw std::runtime_error("Priors Input Format Error: The first probeset prior name must be \"GENERIC\"!\n");
        if( ipriors[0].size() < 2 )
            throw std::runtime_error("Priors Input Format Error: The GENERIC prior must have two models \n (one is for copynumber one, the other is for copynumber two)!\n");
        /* Updating priors */
        NIW_generic_prior = ipriors[0];
        if(( ipriors.size() == 1 ))
        {
            initialize_by_copynumber_categories( ipriors, NIW_generic_prior );
            ipriors.swap   ( NIW_priors );
        }
        else 
        {
            initialize_by_copynumber_categories( NIW_priors, NIW_generic_prior );
            const auto& input2cube_order = idmapping( iprobeset_names, cube_probeset_names );
            for( auto id = 0; id < probesets_num; id++ )
            {
                auto& ref = ipriors[ input2cube_order[ id ] ];
                if( auto it = ref.begin(); it->second.name == "GENERIC" ) continue;
                
                for( auto& [ spec_cn, mdl ]: NIW_priors[ id ] )
                { 
                    if( auto it = ref.find( spec_cn ); it != ref.end() )
                        mdl = it->second;
                }
            }
        }
    }

    void setup_NIW_bayes_model_basics( const std::string& ipath, const std::vector<std::string>& cube_probeset_names )
    {
        if( cube_probeset_names.empty() )
            throw std::runtime_error("Unexpected probeset name number from the input expression cube.\n");
        ccagt::probeset_param sp{};
        sp.InitializeWithGenericPrior();

        NIW_generic_prior = { { 1, sp.prior }, { 2, sp.prior } };
        initialize_by_copynumber_categories( NIW_priors, NIW_generic_prior );

        if( !ipath.empty() ) 
        {
            if(boost::filesystem::exists( ipath )) 
            {
                read_NIW_priors( ipath, cube_probeset_names );
            }
        }
        NIW_posteriors = NIW_priors;
    }

    void save_NIW_posteriors( const std::string& opath )
    {
        std::vector< std::map< int, ccagt::probeset_distribution > > oposteriors = { NIW_generic_prior };
        oposteriors.insert(
            oposteriors.end(), 
            NIW_posteriors.begin(), 
            NIW_posteriors.end()
        );

        std::ofstream ofs ( opath );
        boost::archive::xml_oarchive ar ( ofs );
        ar & boost::serialization::make_nvp("NIW_models",           oposteriors);
    }
};
} // end of data_pool
} // end of engine
} // end of cpt
#else
#endif