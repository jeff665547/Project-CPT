#pragma once
#include <boost/variant.hpp>
#include <map>
#include <CPT/format/general_basic_variant.hpp>
#include <Nucleona/language.hpp>
namespace cpt {
namespace format {
namespace tsv {


using DynamicTuple = std::vector<GeneralBasicVariant>;

template<class ROW>
class TsvEntryTpl 
// : public ROW
{
    const std::map<std::string, std::size_t>& meta_;
    ROW data_;
  public:
    // using Base = ROW;
    using Data = ROW;
    TsvEntryTpl ( const std::map<std::string, std::size_t>& meta )
    : meta_ ( meta )
    {
        if ( data_.size() < meta_.size() )
            data_.resize(meta_.size());
    }
    TsvEntryTpl ( 
        const std::map<std::string, std::size_t>& meta 
        , Data data
    )
    : meta_ ( meta )
    , data_ ( data )
    {
        if ( data_.size() < meta_.size() )
            data_.resize(meta_.size());
    }
    template<class T>
    T& get(const std::string& label)
    {
        return boost::get<T>(data_.at(meta_.at(label)));
    }
    template<class T>
    const T& get(const std::string& label) const
    {
        return boost::get<T>(data_.at(meta_.at(label)));
    }
    auto& get_variant( const std::string& label )
    {
        return data_.at(meta_.at(label));
    }
    const auto& get_variant( const std::string& label ) const 
    {
        return data_.at(meta_.at(label));
    }
/* index version */
    template<class T>
    T& get_by_idx ( const std::size_t& idx )
    {
        return boost::get<T>(data_.at(idx));
    }
    template<class T>
    const T& get_by_idx ( const std::size_t& label ) const
    {
        return boost::get<T>(data_.at(label));
    }
    auto& get_variant_by_idx ( const std::size_t& label )
    {
        return data_.at(label);
    }
    const auto& get_variant_by_idx ( const std::size_t& label ) const 
    {
        return data_.at(label);
    }
    GETTER( auto&, data(), 
    {
        return data_;
    })
};

}}}
