#pragma once
#include <random>
#include <armadillo>
#include <CPT/utility/irange.hpp>
#include <boost_addon/range_vector.hpp>
namespace cpt {
namespace algorithm {
namespace cu = cpt::utility;

class ProbesetSampleRndChoice
{
  public:
    ProbesetSampleRndChoice() = default;
    using RndIds = std::vector<
        std::pair<
              std::size_t /* probeset */
            , std::size_t /* sample */
        >
    >;
    template<class CUBE, class RND_NUMS>
    auto get_rnd_vec(CUBE&& cube, RND_NUMS&& rnd_nums) const
    {
        RndIds res;
        auto& allele_signals = cube;
        auto sample_nums   ( allele_signals.n_cols    );
        auto probeset_nums ( allele_signals.n_slices  );
        if ( rnd_nums < 1 )
        {
            res.reserve(sample_nums * probeset_nums);
            for ( auto i : cu::irange_0(sample_nums) )
                for ( auto j : cu::irange_0( probeset_nums ) )
                    res.emplace_back( i, j );
        }
        else
        {
            std::default_random_engine gen;
            arma::Mat<int> check(sample_nums, probeset_nums, arma::fill::zeros);
            std::uniform_int_distribution<std::size_t> probeset(
                  0
                , probeset_nums - 1
            );
            std::uniform_int_distribution<std::size_t> sample(
                  0
                , sample_nums - 1
            );
            for( std::size_t i (0); i < rnd_nums; i ++ )
            {
                auto pid = (probeset(gen));
                auto sid = (sample(gen));
                if ( check ( sid, pid ) == 0 )
                {
                    res.emplace_back(pid, sid);
                    check ( sid, pid ) = 1;
                }
                else
                {
                    i --;
                    continue;
                }
            }
        }
        return res;
    }
};
class ProbesetRndChoice
{
  public:
    using RndIds = std::vector<std::size_t>;
    ProbesetRndChoice() = default;

    template<class CUBE, class RND_NUMS>
    auto get_rnd_vec(CUBE&& cube, RND_NUMS&& rnd_nums) const
    {
        RndIds res;
        auto& allele_signals = cube;
        auto probeset_nums ( allele_signals.n_slices  );

        if( rnd_nums < 1 )
        {
            res.reserve(probeset_nums);
            for ( auto i : cu::irange_0(probeset_nums) )
            {
                assert ( i <= (decltype(i))(std::numeric_limits<std::size_t>::max() ) );
                res.emplace_back( i );
            }
        }
        else
        {
            std::default_random_engine gen;
            std::uniform_int_distribution<std::size_t> probeset( 0, 65535 );
            auto rnd_tab ( cu::irange_0(probeset_nums) | ::to_vector );
            auto last = probeset_nums ;
            for( std::size_t i (0); i < rnd_nums; i ++ )
            {
                auto pid = (probeset(gen) % last); /* random in some range */
                res.emplace_back(rnd_tab[pid]);
                std::swap ( rnd_tab[pid], rnd_tab[last - 1]);
                last --;
            }
        }
        return res;
    }
};

}}
