#pragma once
/* row major version implement */

#include <CPT/format/general_basic_variant.hpp>
#include <Nucleona/language.hpp>
#include <CPT/const_tags.hpp>
#include <mutex>
#include <CPT/format/tsv/data_ref.hpp>
namespace cpt {
namespace format {
namespace tsv {

template< 
      template<class T, class... RT> class ROW_CONTAINER
    , template<class T, class... RT> class COL_CONTAINER
    , class PARALLEL
>
class RowMajorModel
{
  public: 
    using ColRef = DataRef;
    using RowRef = COL_CONTAINER<GeneralBasicVariant>&;
  private:
    // using Col = ColRef;
    ROW_CONTAINER<COL_CONTAINER<GeneralBasicVariant>> content_;
    std::size_t col_num_ {0};
    auto add_row_base( 
          ROW_CONTAINER<GeneralBasicVariant>&& ds 
        , ROW_CONTAINER<GeneralBasicVariant>*& res
    ) noexcept
    {
        content_.emplace_back(std::move(ds));
        res = &content_.back();
    }
    auto add_col_base( ColRef& col_ref ) noexcept
    {
        for( auto& row : content_ )
        {
            row.emplace_back(GeneralBasicVariant());
            col_ref.emplace_back( row.back() );
        }
        col_num_ ++;
    }
  public:
    GETTER(auto&, get ( const std::size_t& i, const std::size_t& j ),
    {
        return content_.at(i).at(j);
    })

    GETTER( RowRef, row ( const std::size_t& i ),
    {
        return content_.at(i);
    })
    // RowRef row( const std::size_t& i )
    // {
    //     return content_.at(i);
    // }
        
    GETTER( ColRef, col ( const std::size_t& j ),
    {
        ColRef colr;
        for( auto& rowr : content_ )
        {
            colr.emplace_back( rowr.at(j) );
        }
        return colr;
    })
    ColRef add_column() noexcept
    {
        ColRef col_ref;
        if constexpr ( std::is_same<cpt::NoParallel, PARALLEL>::value )
        {
            this->add_col_base(col_ref);
        }
        else // TODO: parallel version
        {
            throw std::logic_error( "Not implement." );
        }
        return col_ref;
    }
    RowRef add_row() noexcept
    {
        ROW_CONTAINER<GeneralBasicVariant>* res ( nullptr );
        ROW_CONTAINER<GeneralBasicVariant> ds;
        ds.resize(col_num_);
        if constexpr( std::is_same<cpt::NoParallel, PARALLEL>::value )
        {
            this->add_row_base( std::move(ds), res );
        }
        else // parallel version 
        {
            throw std::logic_error( "Not implement." );
        }
        return *res;
    }
    std::size_t row_num() const noexcept
    {
        return content_.size();
    }
    std::size_t col_num() const noexcept
    {
        return col_num_;
    }
    void clear() noexcept
    {
        typedef decltype(content_) ContentType;
        ContentType blank;
        blank.swap(content_);
    }

};

}}}
