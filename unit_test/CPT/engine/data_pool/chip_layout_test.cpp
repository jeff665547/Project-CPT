#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <CPT/engine/data_pool/chip_layout.hpp>
#include <string>
#include <CCD/stream_var.hpp>

namespace ced_ = cpt::engine::data_pool;
using namespace std::string_literals;

template<class E>
bool is_in ( E&& e, const std::vector<E>& l )
{
    for ( auto&& v : l )
    {
        if ( e == v ) return true;
    }
    return false;
}
template<class E>
bool expect_eq_vec( std::vector<E>& v1, std::vector<E>& v2 )
{
    std::sort( v1.begin(), v1.end() );
    std::sort( v2.begin(), v2.end() );

    for ( std::size_t i (0); i < v2.size(); i ++ )
    {
        EXPECT_EQ( v1.at(i), v2.at(i) );
    }
}
TEST( chip_layout, CAD_CDF_compare_test )
{
    ced_::CdfChipLayout cdf;
    ced_::GenotypingCadChipLayout cad;

    cad.read_chip_layout( (nucleona::test::data_dir() / "r2.cad").string() );
    cdf.read_chip_layout( (nucleona::test::data_dir() / "r2.cdf").string() );

    auto cad_nps ( cad.num_probesets() );
    auto cdf_nps ( cdf.num_probesets() );

    std::cout << VDUMP( cad_nps ) << std::endl;
    std::cout << VDUMP( cdf_nps ) << std::endl;

    EXPECT_EQ( cad_nps, cdf_nps );

    auto&& cad_npps ( cad.num_probes() );
    auto&& cdf_npps ( cdf.num_probes() );
    EXPECT_EQ( cad_npps, cdf_npps );
    std::size_t cad_sum_nps(0);
    std::size_t cdf_sum_nps(0);

    auto&& psl = cad.cad_.get_probeset_list();
    for ( std::size_t i (0); i < cdf_nps; i ++ )
    {
        auto cad_np( cad.num_probes( i ) );
        auto cdf_np( cdf.num_probes( i ) );
        cad_sum_nps += cad_np;
        cdf_sum_nps += cdf_np;

        EXPECT_EQ( cad_np, cdf_np );
        
        std::vector<char> cad_alleles;
        std::vector<std::size_t> cad_channel;
        std::vector<std::size_t> cad_probe;

        std::vector<char> cdf_alleles;
        std::vector<std::size_t> cdf_channel;
        std::vector<std::size_t> cdf_probe;

        auto&& ps = psl.at(i);
        auto&& pl = ps.get_probe_list();
        auto plsize = pl.size();
        // assert ( plsize == cdf_np );
        for ( std::size_t j (0); j < plsize; j ++ )
        {
            auto&& p  = pl.at(j);
            auto&& cd = cpt::format::make_json( p.channel_des() );
            auto rc = cd.template get<std::string>("channel");
            auto ra = cd.template get<std::string>("allele");
            auto cl ( cad.split_t<std::size_t>( rc ) );
            auto al ( cad.split_t<char> ( ra ) );
            assert ( cl.size() == al.size() );
            for ( std::size_t i(0); i < cl.size(); i ++ )
            {
                cad_channel.emplace_back( cl.at(i) );
                cad_alleles.emplace_back( al.at(i) );
                cad_probe.emplace_back( p.probe_name() );
            }
            
        }
        for ( std::size_t j(0); j < cdf_np; j ++ )
        {
            cdf_channel.emplace_back( cdf.probe_channel( i, j ));
            cdf_alleles.emplace_back( cdf.probe_allele( i, j ));
            cdf_probe.emplace_back( cdf.probe(i, j ) );

            // auto&& cad_p = cad.probe( i, j );
            // auto&& cad_a = cad.probe_allele( i, j );
            // auto&& cad_c = cad.probe_channel( i, j );
            // cad_alleles.emplace_back( cad_a );
            // cad_probe.emplace_back( cad_p );
            // cad_channel.emplace_back( cad_c );

            // auto&& cdf_p = cdf.probe( i, j );
            // auto&& cdf_a = cdf.probe_allele( i, j );
            // auto&& cdf_c = cdf.probe_channel( i, j );
            // cdf_alleles.emplace_back( cdf_a );
            // cdf_probe.emplace_back( cdf_p );
            // cdf_channel.emplace_back( cdf_c );

            // assert( p != std::size_t(-1) );

            // assert( is_in( cad.probe_allele( i, j ), { 'A', 'B', 'C', 'D' } ));
            // assert( is_in( cad.probe_channel( i, j ), { 0, 1, 2, 3 } ));
        }
        expect_eq_vec( cad_alleles, cdf_alleles );
        expect_eq_vec( cad_channel, cdf_channel );
        expect_eq_vec( cad_probe, cdf_probe );
    }
    EXPECT_EQ( cad_sum_nps, cdf_sum_nps );
}
