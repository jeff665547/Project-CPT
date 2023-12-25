#pragma once
#include <mlpack/methods/gmm/gmm.hpp>
#include <algorithm>
#include <cmath>
#include <armadillo>
#include <CPT/range.hpp>
#include <boost/range/combine.hpp>
#include <CPT/algorithm/g_m_m_util/access.hpp>
#include <CPT/algorithm/dist_constrain/bic.hpp>
#include <CPT/algorithm/dist_constrain/fan.hpp>
#include <CPT/algorithm/dist_constrain/fan2.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <CPT/logger.hpp>
namespace cpt {
namespace component {
namespace birdseed {
RANGE_NAMESPACE_SHORTCUT
namespace mg = mlpack::gmm;
namespace cagu = ca::g_m_m_util;
namespace cadc = cpt::algorithm::dist_constrain;
template<class MDL, class OB>
struct ConstrainData
{
    MDL mdl;
    #ifdef PROBESET_TRAINING_PLOT
    OB ob;
    #endif
    double bic;
    double fan;
    ConstrainData ( 
        const MDL& _mdl
        #ifdef PROBESET_TRAINING_PLOT
        , const OB& _ob
        #endif
        , double _bic = 0.0
        , double _fan = 0.0 
    )
    : mdl ( _mdl )
    #ifdef PROBESET_TRAINING_PLOT
    , ob  ( _ob  )
    #endif
    , bic ( _bic )
    , fan ( _fan )
    {}
    ConstrainData() = default;
};
template<class MDL, class OB, class LOG>
struct IDistConstrain
{
    using This = IDistConstrain<MDL,OB,LOG>;
    using Elem = ConstrainData<MDL, OB>;
    virtual void score( MDL& model, OB& ob, LOG& logger ) = 0;
    virtual std::tuple<bool, Elem> get_best( LOG& logger ) = 0;
    virtual std::unique_ptr<This> clone() = 0;
};
template<class MDL, class OB, class METRIC, class LOG>
struct DistConstrainSimpleBic : public IDistConstrain < MDL, OB, LOG >
{
    using Elem = ConstrainData<MDL, OB>;
    using Base = IDistConstrain < MDL, OB, LOG >;
    using This = DistConstrainSimpleBic< MDL, OB, METRIC, LOG>;
    Elem best_result;

    
    DistConstrainSimpleBic( LOG& logger)
    : best_result(
          MDL()
        #ifdef PROBESET_TRAINING_PLOT
        , OB()
        #endif
        , std::numeric_limits<double>::max()
    )
    {}
    virtual void score( MDL& model, OB& ob, LOG& logger ) override
    {
        auto score = cadc::Bic2<METRIC>::score(model, ob);
        // auto score = cadc::Bic2<cadc::GMM>::score(model, ob);
        if ( best_result.bic > score)
        {
            best_result.bic = score;
            best_result.mdl = model;
            #ifdef PROBESET_TRAINING_PLOT
            best_result.ob = ob;
            #endif
        }
        logger << "bic : " << score << std::endl;
    }
    virtual std::tuple<bool, Elem> get_best(LOG& logger) override
    {
        return std::make_tuple( true, best_result );
    }
    virtual std::unique_ptr<Base> clone() override
    {
        return std::unique_ptr<Base>(new This(*this));
    }
};
template<class MDL, class OB, class METRIC, class LOG>
struct DistConstrainSimpleFan2 : public IDistConstrain < MDL, OB, LOG >
{
    using Elem = ConstrainData<MDL, OB>;
    using Base = IDistConstrain < MDL, OB, LOG >;
    using This = DistConstrainSimpleFan2< MDL, OB, METRIC, LOG>;
    Elem best_result;
    cadc::Fan2<> fan2;

    
    DistConstrainSimpleFan2( LOG& logger)
    : best_result(
          MDL()
        #ifdef PROBESET_TRAINING_PLOT
        , OB()
        #endif
        , std::numeric_limits<double>::max()
        , std::numeric_limits<double>::min()
    )
    {}
    virtual void score( MDL& model, OB& ob, LOG& logger ) override
    {
        auto score = fan2.score(model, ob, logger);
        if ( best_result.fan < score) /* bigger is better */
        {
            best_result.fan = score;
            best_result.mdl = model;
            #ifdef PROBESET_TRAINING_PLOT
            best_result.ob = ob;
            #endif
        }
        logger << "fan2 : " << score << std::endl;
    }
    virtual std::tuple<bool, Elem> get_best(LOG& logger) override
    {
        return std::make_tuple( true, best_result );
    }
    virtual std::unique_ptr<Base> clone() override
    {
        return std::unique_ptr<Base>(new This(*this));
    }
};
template<class MDL, class OB, class METRIC, class LOG>
struct DistConstrain2 : public IDistConstrain<MDL, OB, LOG>
{
    using Elem = ConstrainData<MDL, OB>                     ;
    using This = DistConstrain2<MDL, OB, METRIC, LOG>       ;
    using Base = IDistConstrain<MDL, OB, LOG>               ;
    std::vector<std::size_t>    bic_cset                    ;
    std::vector<std::size_t>    fan_cset                    ;
    std::size_t                 conserv_n                   ;
    std::vector<Elem>           olist                       ;
    cadc::Fan2<>                 fanf                       ;
    
    DistConstrain2(
          const std::size_t& _conserv_n 
        , const std::size_t& n_candidates
        , LOG              & logger
    )
    : conserv_n ( _conserv_n    )
    {
        olist   .reserve( n_candidates );
        bic_cset.reserve( n_candidates );
        fan_cset.reserve( n_candidates );
    }
    virtual void score( MDL& model, OB& ob, LOG& logger ) override
    {
        auto bic = cadc::Bic2<METRIC>::score  ( model, ob );
        auto fan = fanf.score( model, ob, logger );
        olist.emplace_back(
              model
        #ifdef PROBESET_TRAINING_PLOT
            , ob
        #endif
            , bic
            , fan
        );
        bic_cset.emplace_back( olist.size() - 1 );
        fan_cset.emplace_back( olist.size() - 1 );
        logger << "fan : " << fan << std::endl;
        logger << "bic : " << bic << std::endl;
    }
    virtual std::tuple<bool, Elem> get_best(LOG& logger) override
    {
        Elem constrain_result;
        std::vector<std::size_t> inter;
        std::sort( bic_cset.begin(), bic_cset.end(), [this]( auto&& e1, auto&& e2 )
        {
            return olist.at(e1).bic < olist.at(e2).bic;
        });
        std::sort( fan_cset.begin(), fan_cset.end(), [this]( auto&& e1, auto&& e2 )
        {
            return olist.at(e1).fan > olist.at(e2).fan;
        });
        bic_cset.resize(conserv_n);
        fan_cset.resize(conserv_n);
        show_bics(logger);
        show_fans(logger);
        for ( auto&& b : bic_cset )
        {
            for ( auto&& f : fan_cset )
            {
                if ( b == f )
                {
                    inter.push_back(b);
                }
            }
        }
        logger << "pass number : " << inter.size() << std::endl;
        if ( inter.size() > 0 )
        {
            constrain_result = olist.at(inter.at(0));
            logger << "pick score : " << constrain_result.bic << ',' << constrain_result.fan << std::endl;
            return std::make_tuple( true, std::move(constrain_result) );
        }
        else return std::make_tuple( false, std::move(constrain_result) );
    }
    virtual std::unique_ptr<Base> clone() override
    {
        return std::unique_ptr<Base>(new This(*this));
    }
    auto show_bics(LOG& logger)
    {
        logger << "bic in list : ";
        for ( auto&& i : bic_cset )
            logger << olist.at(i).bic << '\t' << std::endl;
    }
    auto show_fans(LOG& logger)
    {
        logger << "fan in list : ";
        for ( auto&& i : fan_cset )
            logger << olist.at(i).fan << '\t' << std::endl;
    }
};
template<class MDL, class OB, class METRIC, class LOG>
struct DistConstrain : public IDistConstrain<MDL, OB, LOG>
{
    using Elem = ConstrainData<MDL, OB>                     ;
    using This = DistConstrain<MDL, OB, METRIC, LOG>        ;
    using Base = IDistConstrain<MDL, OB, LOG>               ;
    std::vector<std::size_t>    bic_cset                    ;
    std::vector<std::size_t>    fan_cset                    ;
    std::size_t                 conserv_n                   ;
    std::vector<Elem>           olist                       ;
    cadc::Fan<>                 fanf                        ;
    
    template<class GM>
    DistConstrain(
          const std::size_t& _conserv_n 
        , const std::size_t& n_candidates
        , const GM         & gm
        , LOG              & logger
    )
    : conserv_n ( _conserv_n    )
    , fanf      ( gm, logger    )
    {
        olist   .reserve( n_candidates );
        bic_cset.reserve( n_candidates );
        fan_cset.reserve( n_candidates );
    }
    virtual void score( MDL& model, OB& ob, LOG& logger ) override
    {
        auto bic = cadc::Bic2<METRIC>::score  ( model, ob );
        auto fan = fanf.score( model, ob, logger );
        olist.emplace_back(
              model
        #ifdef PROBESET_TRAINING_PLOT
            , ob
        #endif
            , bic
            , fan
        );
        bic_cset.emplace_back( olist.size() - 1 );
        fan_cset.emplace_back( olist.size() - 1 );
        logger << "fan : " << fan << std::endl;
        logger << "bic : " << bic << std::endl;
    }
    virtual std::tuple<bool, Elem> get_best(LOG& logger) override
    {
        Elem constrain_result;
        std::vector<std::size_t> inter;
        std::sort( bic_cset.begin(), bic_cset.end(), [this]( auto&& e1, auto&& e2 )
        {
            return olist.at(e1).bic < olist.at(e2).bic;
        });
        std::sort( fan_cset.begin(), fan_cset.end(), [this]( auto&& e1, auto&& e2 )
        {
            return olist.at(e1).fan < olist.at(e2).fan;
        });
        bic_cset.resize(conserv_n);
        fan_cset.resize(conserv_n);
        show_bics(logger);
        show_fans(logger);
        for ( auto&& b : bic_cset )
        {
            for ( auto&& f : fan_cset )
            {
                if ( b == f )
                {
                    inter.push_back(b);
                }
            }
        }
        logger << "pass number : " << inter.size() << std::endl;
        if ( inter.size() > 0 )
        {
            constrain_result = olist.at(inter.at(0));
            logger << "pick score : " << constrain_result.bic << ',' << constrain_result.fan << std::endl;
            return std::make_tuple( true, std::move(constrain_result) );
        }
        else return std::make_tuple( false, std::move(constrain_result) );
    }
    virtual std::unique_ptr<Base> clone() override
    {
        return std::unique_ptr<Base>(new This(*this));
    }
    auto show_bics(LOG& logger)
    {
        logger << "bic in list : ";
        for ( auto&& i : bic_cset )
            logger << olist.at(i).bic << '\t' << std::endl;
    }
    auto show_fans(LOG& logger)
    {
        logger << "fan in list : ";
        for ( auto&& i : fan_cset )
            logger << olist.at(i).fan << '\t' << std::endl;
    }
};
// template<class MDL, class OB, class LOG>
// using DistConstrainPtr = std::unique_ptr<IDistConstrain<MDL,OB,LOG>>;
// void test()
// {
//     DistConstrain<mg::GMM, arma::mat> qq(2, 6);
// }

}}}
