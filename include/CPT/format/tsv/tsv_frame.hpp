#pragma once
#include <Nucleona/type_traits/core.hpp>
namespace cpt {
namespace format {
namespace tsv {

template< 
      template< class T, class... R > class ENTRY_CONTAINER 
    , class ENTRY_TYPE
    , class TSV_TYPE
>
class TsvFrameTpl
{
    using EntryType = ENTRY_TYPE;
    using TsvType = TSV_TYPE;
    ENTRY_CONTAINER<EntryType> content_;
    TsvType& tsv_;
  public:
    TsvFrameTpl ( TsvType& tsv )
    : tsv_ ( tsv )
    {
        content_.emplace_back ( EntryType(tsv_.meta() ) );
    }
    template<class T>
    std::size_t set_property( 
          const std::string& label
        , const T& value 
    )
    {
        return set_property_by_idx( tsv_.get_meta_index ( label ), value );
    }
    template<class T>
    std::size_t set_property_by_idx(
          const std::size_t& id
        , const T& value
    )
    {
        if constexpr( nucleona::type_traits::IsSequenceContainer<T>::value )
        {
            ENTRY_CONTAINER<EntryType> data;
            for ( auto& v : value )
            {
                for ( auto& c : content_ )
                {
                    data.emplace_back ( c );
                    data.back().get_variant_by_idx( id ) = v;
                }
            }
            content_.swap(data);
        }
        else
        {
            for ( auto& v : content_ )
            {
                v.get_variant_by_idx( id ) = value;
            }
        }
        return content_.size();
    }
    void flush()
    {
        for ( auto& v : content_ ) 
        {
            // tsv_.emplace_back ( std::move(v) );
            // TODO : add row
            // tsv_.add_row(v);
            tsv_.push_entry(std::move(
                v.data()
            ));
        }
    }
};
}}}
#define FRAME_SET_PROPERTY( frame, obj, mem ) frame.set_property(#mem, obj.mem)
