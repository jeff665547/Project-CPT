#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <CPT/forward.hpp>
#include <Nucleona/language.hpp>
#include <CPT/engine/data_pool/data_paths_pool.hpp>
#include <CPT/format/celfile.hpp>
#include <set>
#include <CPT/logger.hpp>

namespace cpt {
namespace engine {
namespace data_pool {

class CelImpl
{
  protected:
      std::set<std::string> is_load_;
  public:
      cpt::format::CELFile cel;
    template<class DB>
    CelImpl( DB& db ) 
    {
        auto& pipeline_schema ( db.pipeline_schema() );
        for ( auto& child 
                : pipeline_schema.get_child("input").get_child("cel") )
        {
            db.push_path("cel_path", child.second);
        }
    }
    template<class DB>
    void load_cel(const std::string& fname, DB& db)
    {
        cpt::verbose0 << "load " << fname << std::endl;
        auto cel = cpt::format::CELFile::load(fname);
        for (int channel = 0; channel != 2; ++channel)
            db.signal_intansities[channel].emplace_back(cel->extract_intensities(channel));
    }
    template<class DB>
    void require_cel(const std::string& fname, DB& db)
    {
        if( is_load_.find(fname) == is_load_.end() )
        {
            cpt::verbose0 << "load : " << fname << std::endl;
            load_cel ( fname, db );
            is_load_.emplace(fname);
        }
    }
};
using Cel = CelImpl;

} // end of namespace data_pool
} // end of namespace engine
} // end of namespace cpt
