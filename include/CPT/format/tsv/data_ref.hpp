#pragma once

#include <CPT/format/general_basic_variant.hpp>
#include <vector>
#include <Nucleona/language.hpp>
namespace cpt {
namespace format {
namespace tsv {

using DataRefBase = std::vector<cpt::format::GeneralBasicVariant*>;
class DataRef 
: protected DataRefBase
{
    using Base = DataRefBase;
  public: 
    GETTER(auto&, operator[] ( const Base::size_type& idx ), 
    {
        return *Base::operator[]( idx );
    })
    GETTER(auto&, at( const Base::size_type& idx ),
    {
        return *Base::at(idx);
    })
    template<class T>
    auto emplace_back ( T& v )
    {
        return Base::emplace_back(&v);
    }
    auto size() const noexcept
    {
        return Base::size();
    }
    template<class... T>
    auto resize(T&&... o) noexcept
    {
        return Base::resize(std::forward<T>(o)...);
    }
};

}}}
