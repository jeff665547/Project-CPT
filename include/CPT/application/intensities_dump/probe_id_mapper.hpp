#pragma once
#include <utility>
#include <fstream>
#include <vector>
#include <CPT/format/static_tsv_reader.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <CPT/utility/gprofiler.hpp>
namespace cpt { namespace application {
namespace intensities_dump {
struct ProbeIdMapper
{
    virtual std::size_t get_id( 
          std::size_t x
        , std::size_t y
    ) const = 0;
};
/* dont care point domain */
struct TraditionalIdMapper : public ProbeIdMapper
{
    using size_type = std::size_t;
    TraditionalIdMapper( const size_type& n_col )
    : n_col_( n_col )
    {}
    virtual std::size_t get_id( std::size_t x, std::size_t y ) const override
    {
        return x + y * n_col_;
    }
    // template<class X, class Y>
    // auto operator()( 
    //     const X& x, const Y& y 
    // )
    // {
    //     return x + y * n_col_;
    // }
  private:
    size_type n_col_;
};

struct TabularIdMapper : public TraditionalIdMapper
{
    using Base = TraditionalIdMapper;
    using size_type = std::size_t;
    
    using TsvParserTrait = cpt::format::TraitPrototype< 
          std::tuple
        , cpt::format::PMT<0, size_type>
        , cpt::format::PMT<1, size_type>
        , cpt::format::PMT<2, size_type>
    >;
    using TsvParser = cpt::format::TupleParser< TsvParserTrait >; 

    template<class S>
    TabularIdMapper( 
          S&& sin 
        , size_type n_row
        , size_type n_col
    )
    : TraditionalIdMapper( n_col )
    , n_row_ ( n_row ) 
    , model_ ( n_row * n_col )
    {
        cache_able_section( 
#ifdef WH_SWAP_DEBUG
              "probe_id_map.whswap.archive"
#else
              "probe_id_map.archive"
#endif
            , [&]( auto& bi ) { bi & model_; }
            , [&]()
            {
                cpt::utility::GProfiler prof ( "read_tsv.prof" );
                TsvParser parser("\t");
                std::string line;
                while( std::getline( sin, line ) )
                {
                    auto&& tup ( parser ( line ) );
                    model_.at( Base::get_id( 
#ifdef WH_SWAP_DEBUG
                          std::get<1>( tup )
                        , std::get<0>( tup )
#else
                          std::get<0>( tup )
                        , std::get<1>( tup )
#endif
                    ) ) = std::get<2>( tup )
                    ;
                }
            }
            , [&]( auto& bo ) { bo & model_; }
        );
        // if ( boost::filesystem::exists(fname) && use_cache_debug)
        // {
        //     std::ifstream fin ( fname ); 
        //     boost::archive::binary_iarchive bi (fin);
        //     bi & model_;
        // }
        // else
        // {
        //     {
        //     }
        //     if ( use_cache_debug )
        //     {
        //         std::ofstream fout( fname );
        //         boost::archive::binary_oarchive bo ( fout );
        //         bo & model_;
        //     }
        // }
    }
    // template<class X, class Y>
    // auto& operator()( 
    //     const X& x, const Y& y 
    // )
    // {
    //     return model_.at( 
    //         TraditionalIdMapper::operator()(
    //         x, y 
    //     ));
    // }
    virtual std::size_t get_id( std::size_t x, std::size_t y ) const override
    {
        return model_.at( 
            TraditionalIdMapper::get_id(
            x, y 
        ));
    }
    // template<class X, class Y>
    // const auto& operator()( 
    //     const X& x, const Y& y 
    // ) const 
    // {
    //     return model_.at( 
    //         TraditionalIdMapper::operator()(
    //         x, y 
    //     ));
    // }
  private: 
    size_type n_row_;
    std::vector<size_type> model_;

};

struct DenaliIdMapper
: public TabularIdMapper
{
    template<class S>
    DenaliIdMapper(
          S&& s
        , const uint16_t& n_rows = 1982
        , const uint16_t& n_cols = 358
    )
    : TabularIdMapper ( 
        std::forward<S>(s)
        , n_rows, n_cols
    )
    {}
    // TODO 358 * 1972
};
struct ClariomIdMapper
: public TraditionalIdMapper
{
    ClariomIdMapper(
        const uint16_t& n_cols = 4500
    )
    : TraditionalIdMapper( n_cols )
    {}
    // TODO 358 * 1972
};
struct ZionIdMapper
: public TraditionalIdMapper
{
    ZionIdMapper(
        const uint16_t& n_cols = 158
    )
    : TraditionalIdMapper( n_cols )
    {}
    // TODO 358 * 1972
};

}
}}
