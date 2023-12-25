#pragma once
#include <mlpack/methods/gmm/gmm.hpp>
#include <CPT/algorithm/mdl_components.hpp>
#include <CPT/algorithm/accumulate.hpp>
#include <boost_addon/range_indexed.hpp>
#include <boost/range/irange.hpp>
#include <Nucleona/language.hpp>
#include <CPT/algorithm/matrix_range.hpp>
namespace cpt {
namespace algorithm {
namespace mg = mlpack::gmm;
namespace bt = boost;
namespace ba = bt::adaptors;
namespace md = mlpack::distribution;
namespace ca = cpt::algorithm;
template<class SAMPLE>
auto posteriors(
      const mg::GMM& model
    , SAMPLE&& point
)
{
    return bt::irange(std::size_t(0), model.Gaussians())
        | glue(ba::transformed(
            [&model]( auto&& i ) 
            {
                return ::make_indexed_value(
                      vcopy(i)
                    , model.Component(i)
                );
            }
        ))
        | glue(ba::transformed(
            [&model, po = point]( auto&& p )
            {
                return 
                      model.Probability(po, p.index())
                    / model.Probability(po);
            }
        ))
        | ca::to_arma_vec()
    ;
}
struct PosteriorsTag{};
template<class SAMPLE_RNG, class MODEL>
auto operator|( 
      SAMPLE_RNG&& sample_rng
    , std::tuple<PosteriorsTag, MODEL> potrs
)
{
    auto&& model ( std::get<1>( potrs ) );
    return ( 
        std::forward<SAMPLE_RNG>(sample_rng)
            | glue(ba::transformed(
                [m = cu::mms(std::forward<MODEL>(model))]( const auto& s ) 
                {
                    return posteriors( m.storage, s );
                }
            ))
    )
    ;
}
template<class MODEL>
auto posteriors_range(MODEL&& model)
{
    return std_addon::make_tuple( 
          PosteriorsTag()
        , std::forward<MODEL>(model)
    );
}
}}
