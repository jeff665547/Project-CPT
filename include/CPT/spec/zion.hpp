#pragma once
#include <CPT/spec/zion/image_spec.hpp>
#include <CPT/spec/doc.hpp>
namespace cpt { namespace spec { 

template<class T>
struct ZionDoc : public cpt::spec::Doc< T >
{
    using IDoc = cpt::spec::Doc< T >;
    ZionDoc() 
    {
        auto img_spec = cpt::spec::zion::image_spec_map_by_doc();
        for( auto&& img : img_spec )
        {
            img_index_.emplace( 
                  img.first
                , std::make_pair( 
                      img.second
                    , T()
                )
            );
        }
    }

    ImageInfoId org2id( uint16_t x, uint16_t y ) const override
    { 
        return std::make_pair(
            ( x / (84 - 10) ), ( y / (84 - 10) ) 
        );
    }
    T& get_ext_value( uint16_t x, uint16_t y ) override
    {
        return img_index_.at( { x, y } ).second;
    }
    typename IDoc::ImageIndex& get_map() override
    {
        return img_index_;
    }
    typename ImageSpecMap::mapped_type& get_img_info( uint16_t x, uint16_t y ) override
    {
        return img_index_.at( { x, y } ).first;
    }

protected:
    typename IDoc::ImageIndex img_index_;
};

}}
