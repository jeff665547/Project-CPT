#pragma once
/* column major version implement */

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
class ColumnMajorModel
{
  public: 
    using RowRef = DataRef;
    using ColRef = ROW_CONTAINER<GeneralBasicVariant>&;
  private:
    std::size_t row_num_ {0};
    COL_CONTAINER<ROW_CONTAINER<GeneralBasicVariant>> content_;
    auto add_row_base( RowRef& row_ref ) noexcept
    {
        for( auto& col : content_ )
        {
            col.emplace_back(GeneralBasicVariant());
            row_ref.emplace_back( col.back() );
        }
    }
    auto add_column_base( 
          ROW_CONTAINER<GeneralBasicVariant>&& ds 
        , ROW_CONTAINER<GeneralBasicVariant>* res
    ) noexcept
    {
        content_.emplace_back(std::move(ds));
        res = &content_.back();
    }
  public:
    GETTER(auto&, get ( const std::size_t& i, const std::size_t& j ) ,
    {
        return content_.at(j).at(i);
    })
    GETTER(RowRef, row ( const std::size_t& i ), 
    {
        RowRef rowr;
        for( auto& colr : content_ )
        {
            rowr.emplace_back( colr.at(i) );
        }
        return rowr;
    })
    GETTER(ROW_CONTAINER<GeneralBasicVariant>&, col ( const std::size_t& j ), 
    {
        return content_.at(j);
    })

    auto& add_column() noexcept
    {
        ROW_CONTAINER<GeneralBasicVariant>* res ( nullptr );
        ROW_CONTAINER<GeneralBasicVariant> ds;
        ds.resize(row_num_);
        if constexpr ( std::is_same<cpt::NoParallel, PARALLEL>::value )
        {
            this->add_column_base( std::move(ds), res );
        }
        else // TODO: parallel version
        {
            throw std::logic_error( "Not implement." );
        }
        return *res;
    }
    auto add_row() noexcept
    {
        RowRef row_ref;
        if constexpr( std::is_same<cpt::NoParallel, PARALLEL>::value )
        {
            this->add_row_base(row_ref);
            row_num_ ++;
        }
        else//TODO: parallel version 
        {
            throw std::logic_error( "Not implement." );
        }
        return row_ref;
    }
    std::size_t row_num() const noexcept
    {
        return row_num_;
    }
    std::size_t col_num() const noexcept
    {
        return content_.size();
    }
    void clear() noexcept
    {
        typedef decltype(content_) ContentType;
        ContentType blank;
        blank.swap(content_);
    }
};

}}}
