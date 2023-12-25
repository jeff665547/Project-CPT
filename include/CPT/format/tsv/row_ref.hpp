#pragma once

#include <CPT/format/general_basic_variant.hpp>
#include <vector>
namespace cpt {
namespace format {
namespace tsv {

using RowRefBase = std::vector<cpt::format::GeneralBasicVariant*>;
class RowRef 
: public RowRefBase
{
    using Base = RowRefBase;
  public: 
    auto& operator[] ( const Base::size_type& idx )
    {
        return *Base::operator[]( idx );
    }
};

}}}
