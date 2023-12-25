#pragma once
#include <boost/archive/xml_oarchive.hpp>
#include <CPT/format/cad.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <Nucleona/language.hpp>
#include <boost/filesystem.hpp>
#include <CPT/engine/data_pool/probeset_table.hpp>
#include <Nucleona/algo/split.hpp>
#include <CPT/format/json.hpp>
#include <boost/lexical_cast.hpp>

namespace cpt { namespace engine { 
namespace data_pool {

namespace cf_ = cpt::format;
namespace bf_ = boost::filesystem;
namespace ced_ = cpt::engine::data_pool;

class ChipLayoutImpl
{
  public:
    virtual void        read_chip_layout    ( const std::string& path )     = 0;
    virtual std::size_t num_probesets       ()                              = 0;
    virtual bool        is_probeset_empty   ()                              = 0;
    virtual std::string probeset_name       ( const std::size_t& id )       = 0;
    virtual std::size_t num_probes          ( const std::size_t& pset_id )  = 0;
    virtual std::size_t num_probes          ()                              = 0;
    virtual uint16_t    num_cols            ()                              = 0;
    virtual uint16_t    num_rows            ()                              = 0;

    virtual char        probe_allele        ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) = 0;

    virtual std::size_t probe_channel       ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) = 0;

    virtual std::size_t probe               (
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) = 0;

    virtual ~ChipLayoutImpl                 () {}
};
class GenotypingCadChipLayout : public ChipLayoutImpl, public ProbesetTable
{
    // using Probe         = cfc_::Probe;
    // using ProbesetType  = cfc_::ProbesetType<Probe>;
    // using Probeset      = cfc_::Probeset<Probe, ProbesetType>;
    // using Cad           = cf_::Cad<Probeset>;

  public:
    cf_::Cad cad_;
    template<class T>
    auto split_t ( const std::string& ppv )
    {
        std::vector<T> res;
        auto list = nucleona::algo::split( ppv, "/" );
        for( auto v : list )
        {
            res.emplace_back( boost::lexical_cast<T> ( v ) );
        }
        return res;
    
    }
    template<class T, class FUNC>
    auto build_probe_property_list( const std::size_t& pid, cf_::cad::ProbeListView& pl, FUNC&& pp, const std::string& ppn )
    {
        std::vector<T> list;
        for ( std::size_t i (0); i < pl.size(); i ++ )
        {
            auto&& p  = pl.at(i);
            auto&& cd = cf_::make_json( pp ( p) );
            auto ppv = cd.template get<std::string>(ppn);
            auto vl ( split_t<T>( ppv ) );
            for ( std::size_t i(0); i < vl.size(); i ++ )
            {
                list.emplace_back( vl.at(i) );
            }
        }
        return list;
    }
    auto build_all_channels( const std::size_t& pid, cf_::cad::ProbeListView& pl ) 
    {
        return build_probe_property_list<std::size_t>( 
              pid, pl
            , []( cf_::cad::Probe& p ) { 
                return p.channel_des();
            }
            , "channel"
        );

    }
    auto build_all_alleles( const std::size_t& pid, cf_::cad::ProbeListView& pl ) 
    {
        return build_probe_property_list<char>( 
              pid, pl
            , []( cf_::cad::Probe& p ) { 
                return p.channel_des();
            }
            , "allele"
        );

    }
    auto get_probe_name( const std::size_t& pid, cf_::cad::ProbeListView& pl ) 
    {
        std::size_t j = 0;
        for ( std::size_t i (0); i < pl.size(); i ++ )
        {
            auto&& p  = pl.at(i);
            auto&& cd = cf_::make_json( p.channel_des() );
            auto ppv = cd.template get<std::string>("channel");
            
            auto vl ( split_t<char>( ppv ) );
            if ( pid < j + vl.size() )
                return p.probe_name();
            else
                j += vl.size();
        }
        throw std::logic_error( "bad pid" );
        // return decltype(pl.at(0).probe_name())(-1);
    }
  public:    
    GenotypingCadChipLayout() = default;
    virtual void read_chip_layout ( const std::string& path )               override
    {
        cad_.read_cad( path, H5F_ACC_RDONLY );
        auto&& psl = cad_.get_probeset_list();
        auto&& pslsize = psl.size();
        this->probeset_table.reserve( pslsize );
        for ( std::size_t i (0); i < pslsize; i ++ )
        {
            cpt::engine::data_pool::Probeset dps;
            auto&& ps = psl.at(i);
            auto&& pl = ps.get_probe_list();
            dps.probeset_id = ps.name();
            for ( std::size_t j (0); j < pl.size(); j ++ )
            {
                auto&& p  = pl.at(j);
                auto&& cd = cpt::format::make_json( p.channel_des() );
                auto rc = cd.get<std::string>("channel");
                auto ra = cd.get<std::string>("allele");
                auto cl ( split_t<std::size_t>( rc ) );
                auto al ( split_t<char> ( ra ) );
                assert ( cl.size() == al.size() );
                for ( std::size_t i(0); i < cl.size(); i ++ )
                {
                    dps.probes.emplace_back( p.probe_name() );
                    dps.alleles.emplace_back( al.at(i) );
                    dps.channels.emplace_back( cl.at(i) );
                }
            }
            this->probeset_table.emplace_back( std::move( dps ) );
        }
    }
    virtual std::size_t num_probesets       ()                              override 
    {
        return probeset_table.size();
    }
    virtual bool        is_probeset_empty   ()                              override 
    {
        return probeset_table.empty();
    }
    virtual std::string probeset_name       ( const std::size_t& id )       override 
    {
        return probeset_table[ id ].probeset_id;
    }
    virtual std::size_t num_probes          ( const std::size_t& pset_id )  override 
    {
        return probeset_table[ pset_id ].probes.size();
    }
    virtual std::size_t num_probes          ()                              override
    {
        return cad_.num_rows() * cad_.num_cols();
    }
    virtual char        probe_allele        ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) override 
    {
        return probeset_table[ pset_id ].alleles[ pid ];
    }

    virtual std::size_t probe_channel             ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) override 
    {
        return probeset_table[ pset_id ].channels[ pid ];
    }
    virtual std::size_t probe (  
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) override
    {
        return probeset_table[ pset_id ].probes[ pid ];
    }
    virtual uint16_t num_cols() override
    {
        return cad_.num_cols();
    }
    virtual uint16_t num_rows() override
    {
        return cad_.num_rows();
    }
    // virtual std::size_t num_probesets       ()                              override 
    // {
    //     return cad_.num_probesets();
    //     // return cad_.probeset_list.size();
    // }
    // virtual bool        is_probeset_empty   ()                              override 
    // {
    //     return cad_.num_probesets() == 0;
    //     // return cad_.probeset_list.empty();
    // }
    // virtual std::string probeset_name       ( const std::size_t& pset_id )       override 
    // {
    //     auto&& pl ( cad_.get_probeset_list() );
    //     auto&& ps ( pl.at( pset_id ) );
    //     return ps.name();
    //     // return cad_.probeset_list[pset_id].name;
    // }
    // virtual std::size_t num_probes          ( const std::size_t& pset_id )  override 
    // {
    //     auto&& psl  ( cad_.get_probeset_list()  );
    //     auto&& ps   ( psl.at( pset_id )         );
    //     auto&& pl   ( ps.get_probe_list()       );

    //     std::size_t j = 0;
    //     for ( std::size_t i (0); i < pl.size(); i ++ )
    //     {
    //         auto&& p  = pl.at(i);
    //         auto&& cd = cf_::make_json( p.channel_des() );
    //         auto&& ppv = cd.template get<std::string>("channel");
    //         auto vl ( split_t<std::size_t>( ppv ) );
    //         j += vl.size();
    //     }
    //     return j;
    // }
    // virtual std::size_t num_probes          ()                              override
    // {
    //     return cad_.num_rows() * cad_.num_cols();
    //     // auto&& psl  ( cad_.get_probeset_list()  );
    //     // std::size_t sum(0);
    //     // for ( std::size_t i (0); i < psl.size(); i ++ )
    //     // {
    //     //     sum += num_probes( i );
    //     // }
    //     // return sum;
    // }
    // virtual char        probe_allele        ( 
    //       const std::size_t& pset_id
    //     , const std::size_t& pid 
    // ) override 
    // {
    //     
    //     auto&& psl  ( cad_.get_probeset_list() );
    //     auto&& ps   ( psl.at( pset_id ) );
    //     auto&& pl   ( ps.get_probe_list() );
    //     // TODO tabluar optimize
    //     auto allele_list ( build_all_alleles ( pid, pl ) );
    //     return allele_list.at( pid );
    // }

    // virtual std::size_t probe_channel             ( 
    //       const std::size_t& pset_id
    //     , const std::size_t& pid 
    // ) override 
    // {
    //     auto&& psl  ( cad_.get_probeset_list() );
    //     auto&& ps   ( psl.at( pset_id ) );
    //     auto&& pl   ( ps.get_probe_list() );
    //     // TODO tabluar optimize
    //     auto channel_list ( build_all_channels ( pid, pl ) );
    //     return channel_list.at( pid );
    //     
    // }
    // virtual std::size_t probe( // probe name
    //       const std::size_t& pset_id
    //     , const std::size_t& pid 
    // ) override 
    // {
    //     auto&& psl  ( cad_.get_probeset_list() );
    //     auto&& ps   ( psl.at( pset_id ) );
    //     auto&& pl   ( ps.get_probe_list() );
    //     auto&& p    ( pl.at( pid ) );
    //     return get_probe_name( pid, pl );
    // }
    // virtual uint16_t num_cols() override
    // {
    //     return cad_.num_cols();
    //     // return cad_.num_cols;
    // }

    // virtual uint16_t num_rows() override
    // {
    //     return cad_.num_rows();
    //     // return cad_.num_rows;
    // }
};
class CdfChipLayout : public ChipLayoutImpl, public ced_::ProbesetTable
{
    uint16_t num_cols_;
    uint16_t num_rows_;
  public:
    CdfChipLayout() = default;
    template<class DB>
    CdfChipLayout( DB& db )
    : ChipLayoutImpl()
    , ced_::ProbesetTable( db )
    {}
    virtual void read_chip_layout( const std::string& path )                override
    {
        std::ifstream f( path, std::ios::binary );
        cpt::format::CDFFile cdf;
        cdf.open( f );
        f.close();
        num_cols_ = cdf.num_cols;
        num_rows_ = cdf.num_rows;
        for( auto& unit : cdf.units )
        {
            cpt::engine::data_pool::Probeset probeset( unit );
            probeset_table.push_back( probeset );
        }
    }
    virtual std::size_t num_probesets       ()                              override 
    {
        return probeset_table.size();
    }
    virtual bool        is_probeset_empty   ()                              override 
    {
        return probeset_table.empty();
    }
    virtual std::string probeset_name       ( const std::size_t& id )       override 
    {
        return probeset_table[ id ].probeset_id;
    }
    virtual std::size_t num_probes          ( const std::size_t& pset_id )  override 
    {
        return probeset_table[ pset_id ].probes.size();
    }
    virtual std::size_t num_probes          ()                              override
    {
        // std::size_t sum (0);
        // for ( std::size_t i(0); i < num_probesets(); i ++ )
        // {
        //     sum += num_probes(i);
        // }
        // return sum;
        return num_cols_ * num_rows_;
    }

    virtual char        probe_allele        ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) override 
    {
        return probeset_table[ pset_id ].alleles[ pid ];
    }

    virtual std::size_t probe_channel             ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) override 
    {
        return probeset_table[ pset_id ].channels[ pid ];
    }
    virtual std::size_t probe (  
          const std::size_t& pset_id
        , const std::size_t& pid 
    ) override
    {
        return probeset_table[ pset_id ].probes[ pid ];
    }
    virtual uint16_t num_cols() override
    {
        return num_cols_;
    }
    virtual uint16_t num_rows() override
    {
        return num_rows_;
    }
};
class ChipLayout
{
    ChipLayoutImpl* impl { nullptr };

  public:    
    ChipLayout() = default;

    template<class DB>
    void read_chip_layout ( DB& db, const std::string& path )
    {
        if ( impl != nullptr )
            delete impl;
        bf_::path p ( path );
        auto ext ( p.extension().string() );
        if ( ext == ".cdf" )
        {
            impl = new CdfChipLayout( db );
        }
        else if ( ext == ".cad" )
        {
            impl = new GenotypingCadChipLayout();
        }
        else
        {
            throw std::logic_error ( "unknown chip layout file extension : " + ext );
        }
        impl->read_chip_layout( path );
    }
    std::size_t num_probesets       ()                             
    {
        return impl->num_probesets();
    }
    bool        is_probeset_empty   ()                             
    {
        return impl->is_probeset_empty();
    }
    std::string probeset_name       ( const std::size_t& id )      
    {
        return impl->probeset_name( id );
    }
    std::size_t num_probes          ( const std::size_t& pset_id ) 
    {
        return impl->num_probes( pset_id );
    }
    std::size_t num_probes()
    {
        return impl->num_probes();
    }

    char        probe_allele        ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    )
    {
        return impl->probe_allele( pset_id, pid );
    }

    std::size_t probe_channel             ( 
          const std::size_t& pset_id
        , const std::size_t& pid 
    )
    {
        return impl->probe_channel( pset_id, pid );
    }
    std::size_t probe (  
          const std::size_t& pset_id
        , const std::size_t& pid 
    )
    {
        return impl->probe( pset_id, pid );
    }
    uint16_t chip_layout_num_cols()
    {
        return impl->num_cols();
    }
    uint16_t chip_layout_num_rows()
    {
        return impl->num_rows();
    }
    ~ChipLayout()
    {
        if ( impl != nullptr )
        {
            delete impl;
        }
    }
};

}
}}

auto compile_test()
{
    cpt::engine::data_pool::ChipLayout chiplayout;

}
