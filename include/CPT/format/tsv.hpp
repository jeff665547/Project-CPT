#pragma once
#include <map>
#include <string>
#include <vector>
#include <boost/variant.hpp>
#include <cinttypes>
#include <sstream>
#include <CPT/forward.hpp>
#include <iostream>
#include <Nucleona/language.hpp>
#include <CPT/format/tsv/tsv_entry.hpp>
#include <CPT/format/tsv/tsv_frame.hpp>
#include <CPT/format/tsv/column_major_model.hpp>
#include <CPT/format/tsv/row_major_model.hpp>
/* Why not use tuple ? */
/* Because the field can't resolve in compiling time, the different component need different field, and they cannot just union because the memory usage */
namespace cpt {
namespace format {
template< 
    class Model
    , template<class T, class... RT> class ROW_ID_CONTAINER = std::vector
    , template<class T, class... RT> class ROW_MUX_CONTAINER = std::vector
    , class MUX = std::mutex 
>
class TsvTpl : public Model
{
    using This = TsvTpl<Model, ROW_ID_CONTAINER, ROW_MUX_CONTAINER, MUX>;
    using TsvRefType = TsvRefTpl<
        ROW_ID_CONTAINER
        , ROW_MUX_CONTAINER
        , This
        , MUX
    >;
    friend TsvRefType;
    std::size_t last_column_;
  public:
    using Base = Model;
    using RowRef = typename Base::RowRef;
    using ColRef = typename Base::ColRef;
    using Meta = std::map<std::string, std::size_t>;
    using EntryType = tsv::TsvEntryTpl<RowRef>;
    using FrameEntry = tsv::TsvEntryTpl<std::vector<GeneralBasicVariant>>;
    using FrameType = tsv::TsvFrameTpl<std::vector, FrameEntry, This>;
  private:
    std::map<std::string, std::size_t> meta_;
    inline void fill_row(RowRef& entry){}
    template<class T1, class... T>
    inline void fill_row(RowRef& entry, T1&& o1, T&&... o)
    {
        entry[entry.size() - (sizeof...(T) + 1)] = o1;
        fill_row(entry, o...);
    }
  public:
    TsvTpl() 
    : last_column_ ( 0 )
    , meta_ ()
    {}
    CGETTER( auto&, meta(), { return meta_; })
    auto register_column(const std::string& label)
    {
        Base::add_column();
        return meta_.emplace( 
              label
            , last_column_ ++
        );
    }
    auto& get_variant_by_idx( const std::size_t& id, const std::size_t& label )
    {
        return Base::get( id, label );
    }
    const auto& get_variant_by_idx( const std::size_t& id, const std::size_t& label ) const 
    {
        return Base::get( id, label );
    }
    template<class T>
    T& get_by_idx( const std::size_t& id, const std::size_t& label )
    {
        return boost::get<T>(get_variant_by_idx ( id, label ));
    }
    template<class T>
    const T& get_by_idx( const std::size_t& id, const std::size_t& label ) const 
    {
        return boost::get<T>(get_variant_by_idx ( id, label ));
    }
    auto& get_variant( const std::size_t& id, const std::string& label )
    {
        return get_variant_by_idx( id, meta_.at(label) );
    }
    const auto& get_variant( const std::size_t& id, const std::string& label ) const 
    {
        return get_variant_by_idx( id, meta_.at(label) );
    }
    template<class T>
    T& get( const std::size_t& id, const std::string& label )
    {
        return boost::get<T>(get_variant(id, label));
    }
    template<class T>
    const T& get( const std::size_t& id, const std::string& label ) const 
    {
        return boost::get<T>(get_variant( id, label ));
    }
    template<class... T>
    void push_entry( const T&... o )
    {
        RowRef row ( Base::add_row() ); 
        fill_row ( row, o... );
    }
    void push_entry( const tsv::DynamicTuple& list )
    {
        RowRef row ( Base::add_row() );
        for ( std::size_t i (0); i < list.size(); i ++ )
        {
            row[i] = list[i];
        }
    }
    GETTER(EntryType, create_entry(), 
    {
        RowRef row( Base::add_row() );
        EntryType entry(meta_, row);
        return entry;
    })
    template<class Value>
    EntryType create_entry_if_not_exist( const std::string& label, const Value& value ) 
    {
        auto j = meta_.at(label);
        for ( std::size_t i(0); i < Base::row_num(); i ++ )
        {
            auto tmp ( get_by_idx<Value> ( i, j ) );
            if ( tmp == value )
            {
                RowRef row ( Base::row( i ) );
                EntryType entry(meta_, row);
                return entry;
            }
        }
        auto res = create_entry();
        res.get_variant(label) = value;
        return res;
    }
    template<class Value>
    bool exist( const std::string& label, Value value )
    {
        auto j = meta_.at(label);
        for ( std::size_t i(0); i < Base::row_num(); i ++ )
        {
            auto tmp ( get_by_idx<Value> ( i, j ) );
            if ( tmp == value )
            {
                return true;
            }
        }
        return false;
    }
    TsvRefType ref();
    FrameType create_frame()
    {
        return FrameType ( *this );
    }
    GETTER(auto&, get_meta_index ( const std::string& label ), 
    {
        return meta_.at(label);
    })
};
template<
      template < 
          template < class... E1 > class C1
        , template < class... E2 > class C2
        , class P
      > class MODEL = tsv::RowMajorModel
    , class PARALLEL = cpt::NoParallel
>
using Tsv = TsvTpl<MODEL<std::vector, std::vector, PARALLEL>>;

template<
      template<class T, class... RT> class ROW_ID_CONTAINER
    , template<class T, class... RT> class ROW_MUX_CONTAINER
    , class TSV_TYPE
    , class MUX
>
class TsvRefTpl
{
    using TsvViewType = TsvViewTpl<
          ROW_ID_CONTAINER
        , ROW_MUX_CONTAINER
        , TSV_TYPE
        , MUX
    >;
    using TsvModifierType = TsvModifierTpl<
          ROW_ID_CONTAINER
        , ROW_MUX_CONTAINER
        , MUX
        , TSV_TYPE
    >;
    friend TSV_TYPE;
    using EntryType = typename TSV_TYPE::EntryType;
  protected:
    TSV_TYPE& tsv_;
    ROW_ID_CONTAINER<std::size_t> row_id_container_;    
    std::map<std::string, std::size_t> meta_;

    void fill_meta(){}
    template<class S1, class... Sn>
    void fill_meta( const S1& s1, const Sn&... sn )
    {
        meta_.emplace(s1, tsv_.meta_.at(s1));
        fill_meta(sn...);
    }
    TsvRefTpl( TSV_TYPE& tsv )
    : tsv_ ( tsv )
    {}
    template<class RIC, class META>
    TsvRefTpl( TSV_TYPE& tsv, RIC&& row_id_container, META&& meta )
    : tsv_ ( tsv )
    , row_id_container_ ( std::forward<RIC> ( row_id_container ) )
    , meta_ ( std::forward<META> ( meta ) )
    {}
    TsvRefTpl( const TsvRefTpl& o ) = default;
    TsvRefTpl() = default;
    TsvRefTpl( TsvRefTpl&& o ) = default;
  public:
    template< template<class T, class... R> class Vec >
    auto& select( const Vec<std::string>& labels )
    {
        for( auto& s : labels )
            meta_.emplace(s, tsv_.meta_.at(s));
        return *this;
    }
    template<class... STR>
    auto& select( const STR&... labels )
    {
        fill_meta(labels...);
        return *this;
    }
    template<class FILTER>
    auto& where(FILTER&& filter)
    {
        for ( auto i(0); i < tsv_.row_num(); i++)
        {
            auto rowr = tsv_.row(i);
            if( filter ( EntryType( tsv_.meta(), rowr ) ) )
            {
                row_id_container_.emplace_back( i );
            }
        }
        return *this;
    }
    TsvViewType view();
    TsvModifierType modify();
};

template<
      template<class T, class... RT> class ROW_ID_CONTAINER
      , template<class T, class... RT> class ROW_MUX_CONTAINER
      , class TSV_TYPE
      , class MUX
>
class TsvViewTpl 
: protected TsvRefTpl < 
    ROW_ID_CONTAINER
    , ROW_MUX_CONTAINER
    , const TSV_TYPE
    , MUX
>
{
  public:
    using Base = TsvRefTpl < 
        ROW_ID_CONTAINER
        , ROW_MUX_CONTAINER
        , const TSV_TYPE 
        , MUX
    >;
    template<class... T>
    TsvViewTpl ( T&&... o )
    : Base ( std::forward<T>(o)... )
    {}
    void dump( std::ostream& out, bool has_meta = true )
    {
        if( has_meta )
        {
            std::stringstream buf;
            for ( auto& p : this->meta_ )
            {
                buf << p.first << '\t';
            }
            auto tmp = buf.str();
            out << tmp.substr(0, tmp.length() - 1) << '\n';
        }
        for( auto& id : this->row_id_container_ )
        {
            std::stringstream buf;
            for ( auto& p : this->meta_ )
            {
                buf << this->tsv_.get_variant( id, p.first ) << '\t';
            }
            auto tmp = buf.str();
            out << tmp.substr(0, tmp.length() - 1) << '\n';
        }
    }
};
template<
      template<class T, class... RT> class ROW_ID_CONTAINER
    , template<class T, class... RT> class ROW_MUX_CONTAINER
    , class MUX
    , class TSV_TYPE
>
class TsvModifierTpl 
:  protected TsvRefTpl < ROW_ID_CONTAINER, ROW_MUX_CONTAINER, TSV_TYPE, MUX >
{
    ROW_MUX_CONTAINER<std::unique_ptr<MUX>> mux_list_;
  public:
    using Base = TsvRefTpl < ROW_ID_CONTAINER, ROW_MUX_CONTAINER, TSV_TYPE, MUX >;
    template<class... T>
    TsvModifierTpl ( T&&... o )
    : Base ( std::forward<T>(o)... )
    {
        mux_list_.reserve(this->row_id_container_.size());
        for ( std::size_t i (0); i < this->row_id_container_.size(); i ++ )
        {
            mux_list_.emplace_back(new std::mutex);
        }
        
    }
    template<class MODFIER>
    auto& apply( MODFIER&& modifier )
    {
        for ( std::size_t i (0); i < this->row_id_container_.size(); i ++ )
        {
            mux_list_[i]->lock();
            typename TSV_TYPE::EntryType entry ( 
                  this->tsv_.meta()
                , this->tsv_.row(this->row_id_container_.at(i)));
            modifier( entry );
            mux_list_[i]->unlock();
        }
        return *this;
    }
    auto view()
    {
        return Base::view();
    }
};

#define TSV_TPL TsvTpl< MODEL, ROW_ID_CONTAINER, ROW_MUX_CONTAINER, MUX >
template< 
      class MODEL
    , template<class T, class... RT> class ROW_ID_CONTAINER 
    , template<class T, class... RT> class ROW_MUX_CONTAINER
    , class MUX 
>
typename TSV_TPL::TsvRefType TSV_TPL::ref()
{
    return typename TSV_TPL::TsvRefType(*this);
}
#undef TSV_TPL
#define TSV_REF_TPL TsvRefTpl<ROW_ID_CONTAINER, ROW_MUX_CONTAINER, TSV_TYPE, MUX>
template<
      template<class T, class... RT> class ROW_ID_CONTAINER
    , template<class T, class... RT> class ROW_MUX_CONTAINER
    , class TSV_TYPE
    , class MUX
>
typename TSV_REF_TPL::TsvViewType TSV_REF_TPL::view()
{
    return typename TSV_REF_TPL::TsvViewType(
          this->tsv_
        , std::move(this->row_id_container_)
        , std::move(this->meta_)
    );
}
template<
      template<class T, class... RT> class ROW_ID_CONTAINER
    , template<class T, class... RT> class ROW_MUX_CONTAINER
    , class TSV_TYPE
    , class MUX
>
typename TSV_REF_TPL::TsvModifierType TSV_REF_TPL::modify()
{
    return typename TSV_REF_TPL::TsvModifierType(std::move(*this));
}
#undef TSV_REF_TPL
}}
