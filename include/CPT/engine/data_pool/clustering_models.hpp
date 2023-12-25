#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/data_pool/clustering_models2.hpp>
#else
#include <CPT/forward.hpp>
// #include <CPT/engine/data_pool.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <mlpack/core.hpp>
#include <mlpack/methods/gmm/gmm.hpp>
#include <CPT/utility/arma_serialization.hpp>
#include <CPT/logger.hpp>

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive& ar, mlpack::gmm::GMM& model, unsigned version)
{
    model.Serialize(ar, version);
}

} // end of serialization
} // end of boost

namespace cpt {
namespace engine {
namespace data_pool {

class ClusteringModels
{
  protected:
    bool loaded_;

  public:
    std::vector<mlpack::gmm::GMM> models; 
    std::vector<bool> validities;

  public:
    template <class DB>
    ClusteringModels(DB& db)
      : loaded_(false)
    {
        auto& root = db.pipeline_schema();
        for (auto& child: root.get_child("input.clustering_models"))
            db.push_path("clustering_models", child.second);
    }

    template <class DB>
    void require_clustering_models(DB& db)
    {
        auto& path = db.get_path("clustering_models");
        cpt::verbose0 << "require model from " << path << std::endl;
        if (!loaded_)
            load_clustering_models(db);
    }

    template <class DB>
    void load_clustering_models(DB& db)
    {
        // cpt::engine::DataPool db;
        auto& path = db.get_path("clustering_models");
        cpt::verbose0 << "import models from " << path << std::endl;
        if (!bfs::exists(path))
            std::runtime_error(path.string() + " not found");
        std::ifstream fs(path.string());
        boost::archive::xml_iarchive ar(fs);
        ar & boost::serialization::make_nvp("ClusteringModels", models);
        ar & boost::serialization::make_nvp("Validities", validities);
        loaded_ = true;
    }

    template <class DB>
    void save_clustering_models(DB& db) const
    {
        auto& path = db.get_path("clustering_models");
        cpt::verbose0 << "export models to " << path << std::endl;
        std::ofstream fs(path.string());
        boost::archive::xml_oarchive ar(fs);
        ar & boost::serialization::make_nvp("ClusteringModels", models);
        ar & boost::serialization::make_nvp("Validities", validities);
    }
    template<class DB>
    auto get_model_path(DB& db) const
    {
        return db.get_path("clustering_models");
        // TODO TEST
    }

    void clear_clustering_models(void)
    {
        if (loaded_)
        {
            std::vector<mlpack::gmm::GMM>().swap(models);
            std::vector<bool>().swap(validities);
            loaded_ = false;
        }
    }
};

} // end of data_pool
} // end of engine
} // end of cpt
#endif
