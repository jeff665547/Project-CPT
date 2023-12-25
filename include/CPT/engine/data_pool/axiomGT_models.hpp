#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/forward.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <CPT/utility/arma_serialization.hpp>
#include <CPT/logger.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/engine/data_pool/axiomGT_auxiliary.hpp>
#include <CPT/components/axiomGT/probeset_models.hpp>

namespace cpt {
namespace engine {
namespace data_pool {
class AxiomGTModel{
  public:
    std::string        ps_name;
    mlpack::gmm::GMM   model;
    bool               validate;
    
    AxiomGTModel()
    : ps_name()
    , model  ()
    , validate( false )
    {}
    AxiomGTModel( const std::string& ps_name, const mlpack::gmm::GMM& gm, const bool& validate )
    : ps_name ( ps_name  )
    , model   (    gm    )
    , validate( validate )
    {}

  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        // std::cout << "In ModelInfo" << std::endl;
        ar & boost::serialization::make_nvp( "ProbesetName",    ps_name );
        ar & boost::serialization::make_nvp( "Model",           model   );
        ar & boost::serialization::make_nvp( "Validate",        validate);
    }
};
} // end of data_pool
} // end of engine
} // end of cpt

namespace boost {
namespace serialization {
namespace ced = cpt::engine::data_pool;
void serialize(
    boost::archive::xml_oarchive& ar, 
    std::pair<const int, ced::AxiomGTModel>& info, 
    unsigned int version
)
{
    // std::cout << "model level output" << std::endl;
    ar & boost::serialization::make_nvp( "Copynumber",            info.first  );
    ar & boost::serialization::make_nvp( "ModelInfo",             info.second );
}

void serialize(
    boost::archive::xml_oarchive& ar, 
    std::map<int, ced::AxiomGTModel>& probeset_info, 
    unsigned int version
)
{
    // std::cout << "probeset level output" << std::endl;
    size_t size = probeset_info.size();
    ar & boost::serialization::make_nvp( "ProbesetInfoCounts",    size        );
    for(auto& info: probeset_info)
    {
        ar & boost::serialization::make_nvp( "Info",              info        );
    }
}

void serialize(
    boost::archive::xml_iarchive& ar, 
    std::pair<int, ced::AxiomGTModel>& info, 
    unsigned int version
)
{
    // std::cout << "model level input" << std::endl;
    ar & boost::serialization::make_nvp( "Copynumber",            info.first  );
    ar & boost::serialization::make_nvp( "ModelInfo",             info.second );
}

void serialize(
    boost::archive::xml_iarchive& ar, 
    std::map<int, ced::AxiomGTModel>& probeset_info, 
    unsigned int version
)
{
    // std::cout << "probeset level input" << std::endl;
    size_t size;
    ar & boost::serialization::make_nvp( "ProbesetInfoCounts",    size        );
    while(size--)
    {
        std::pair<int, ced::AxiomGTModel> info;
        ar & boost::serialization::make_nvp( "Info",              info        );
        probeset_info.insert(info);
    }
}
} // end of serialization
} // end of boost

namespace cpt {
namespace engine {
namespace data_pool {
class AxiomGTModels
{
  protected:
    bool loaded_;
    DataPathsPool path_pool_;

  public:
    std::vector< std::map< int, AxiomGTModel > >   data_likelihood;
    double                                         ocean;
    double                                         maxconfidence;
    AxiomGTAuxiliary                               axiom_gt_aux;

  public:
    template <class DB>
    AxiomGTModels( DB& db )
      : loaded_( false )
    {
        auto& root = db.pipeline_schema();
        for ( auto& child: root.get_child( "context.clustering_models" ) )
            path_pool_.push_path( "clustering_models", child.second );
    }

    template <class DB>
    void require_axiomGT_models( DB& db, const std::vector<std::string>& cube_probeset_names )
    {
        if( cube_probeset_names.empty() )
            throw std::runtime_error("Unexpected probeset name number from the input expression cube.\n");
        std::map< int, AxiomGTModel > default_likelihood = { { 1, AxiomGTModel{} }, { 2, AxiomGTModel{} } };
        axiom_gt_aux.initialize_by_copynumber_categories( data_likelihood, default_likelihood );

        auto& path = path_pool_.get_path( "clustering_models" );
        cpt::verbose0 << "require model from " << path << std::endl;
        if ( !loaded_ )
            load_axiomGT_models( db, cube_probeset_names );
    }

    template <class DB>
    void load_axiomGT_models ( DB& db, const std::vector<std::string>& cube_probeset_names )
    {
        // cpt::engine::DataPool db;
        auto& path = path_pool_.get_path( "clustering_models" );
        cpt::verbose0 << "import models from " << path << std::endl;
        if ( !bfs::exists( path ) )
            std::runtime_error( path.string() + " not found" );
        std::vector< std::map< int, AxiomGTModel > > ilikelihood;
        std::ifstream ifs ( path.string() );
        boost::archive::xml_iarchive ar ( ifs );
        ar & boost::serialization::make_nvp( "AxiomGTModels",     ilikelihood     );
        ar & boost::serialization::make_nvp( "callOcean",         ocean           );
        ar & boost::serialization::make_nvp( "callMaxConfidence", maxconfidence   );
        loaded_ = true;

        std::vector<std::string> ips_names, tmp_names;
        for(const auto& ps_ilikelihood: ilikelihood)
        {
            auto it = ps_ilikelihood.begin();
            ips_names.emplace_back( it->second.ps_name );
        }
        axiom_gt_aux.add_buffer_from_head_for_vec( tmp_names, ips_names );
        const auto& input2cube_order = axiom_gt_aux.idmapping( tmp_names, cube_probeset_names );
        for( auto id = 0; id < cube_probeset_names.size(); id++ )
        {
            if( input2cube_order[ id ] == 0 )
            {
                for( auto& [ spec_cn, mdl ]: data_likelihood[ id ] )
                {
                    mdl.ps_name = cube_probeset_names[ id ];
                }
                continue;
            }

            const auto& true_input_id = input2cube_order[ id ] - 1;

            auto& ref = ilikelihood[ true_input_id ];
            for( auto& [ spec_cn, mdl ]: data_likelihood[ id ] )
            { 
                if( auto it = ref.find( spec_cn ); it != ref.end() )
                    mdl = it->second;
                else
                    mdl.ps_name = cube_probeset_names[ id ];
            }
        }
    }

    template <class DB>
    void save_axiomGT_models( DB& db ) const
    {
        auto& path = path_pool_.get_path( "clustering_models" );
        cpt::verbose0 << "export models to " << path << std::endl;
        std::ofstream fs ( path.string() );
        boost::archive::xml_oarchive ar ( fs );
        ar & boost::serialization::make_nvp( "AxiomGTModels",     data_likelihood );
        ar & boost::serialization::make_nvp( "callOcean",         ocean           );
        ar & boost::serialization::make_nvp( "callMaxConfidence", maxconfidence   );
    }

    template <class DB>
    auto get_axiomGT_models_path( DB& db ) const
    {
        return path_pool_.get_path( "clustering_models" );
    }

    void clear_axiomGT_models( void )
    {
        if (loaded_)
        {
            std::vector<std::map<int, AxiomGTModel>>().swap( data_likelihood );
            ocean = 0.0;
            maxconfidence = 0.0;
            loaded_ = false;
        }
    }
};

} // end of data_pool
} // end of engine
} // end of cpt
#else
#endif
