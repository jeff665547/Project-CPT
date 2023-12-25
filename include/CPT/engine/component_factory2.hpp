#pragma once
#ifdef NEW_DATA_POOL
#include <CPT/engine/component.hpp>
#include <memory>
#include <string>
#include <CPT/components/data_loader.hpp>
#include <CPT/components/auxiliary_params_loader.hpp>
#include <CPT/components/target_sketch_estimation.hpp>
#include <CPT/components/quantile_normalization.hpp>
#include <CPT/components/allele_summarization.hpp>
#include <CPT/components/contrast_centers_stretch_transformation.hpp>
#include <CPT/components/log_ratio_strength_transformation.hpp>
#include <CPT/components/minus_vs_average_transformation.hpp>
#include <CPT/components/genotyping.hpp>
// #include <CPT/components/dummy_allele_summarization.hpp>
// #include <CPT/components/brlmmp/brlmmp_training.hpp>
#include <CPT/components/brlmmp/brlmmp_training_tentative_clustering.hpp>
#include <CPT/components/brlmmp/brlmmp_training_min_bic.hpp>
#include <CPT/components/brlmmp/brlmmp_training_remove_outlier.hpp>
#include <CPT/components/brlmmp/brlmmp_training_grand_model.hpp>
#include <CPT/components/brlmmp/brlmmp_training_imputation.hpp>
#include <CPT/components/brlmmp/brlmmp_training_complete.hpp>
#include <CPT/components/brlmmp/brlmmp_genotyping.hpp>
// #include <CPT/engine/data_pool.hpp>
// #include <CPT/algorithm/birdseed.hpp>
#include <CPT/components/birdseed/grand_model_probeset_choice.hpp>
#include <CPT/components/birdseed/grand_model_training.hpp>
#include <CPT/components/birdseed/log_transform.hpp>
#include <CPT/components/birdseed/probeset_training.hpp>
#include <CPT/components/axiomGT/axiomGT_training.hpp>
#define ID_MAP_TYPE( ID, TYPE ) \
else if ( identifier == ID ) \
{ \
    auto tmp = ComponentPtr( new TYPE(data_pool_, schema_node) ); \
    tmp->config(schema_node); \
    return tmp; \
}
namespace cpt
{
namespace engine
{

class ComponentFactory
{
    template <class T>
    using GenotypingTrainingPhase = component::Genotyping<T, true>;
    template <class T>
    using GenotypingCallingPhase = component::Genotyping<T, false>;

  public : 
    const DataPool& data_pool_;
    ComponentFactory(const DataPool& data_pool)
    : data_pool_ ( data_pool )
    {}
    ComponentPtr create_by_identifier ( 
          const std::string& identifier 
        , const bpt::ptree& schema_node
    )
    {
        if( identifier == "" ){ /* TODO a error handle */ }
        // ID_MAP_TYPE( "quantile_norm"             , component::QuantileNorm   ) 
        // ID_MAP_TYPE( "m_a_plot"                  , component::MAPlot         ) 
        ID_MAP_TYPE( "Input:DataLoader"                         , component::DataLoader                         )  // 0
        // ID_MAP_TYPE( "Dummy:AlleleSummarization"                , component::DummyAlleleSummarization           )
        ID_MAP_TYPE( "Input:AuxiliaryParamsLoader"              , component::AuxiliaryParamsLoader              )  // 1
        ID_MAP_TYPE( "Train:TargetSketchEstimation"             , component::TargetSketchEstimation             )  // 2
        ID_MAP_TYPE( "Transform:QuantileNormalization"          , component::QuantileNormalization              )  // 3
        ID_MAP_TYPE( "Transform:AlleleSummarization"            , component::AlleleSummarization                )  // 4
        ID_MAP_TYPE( "Transform:LogRatioStrength"               , component::LogRatioStrength                   )
        ID_MAP_TYPE( "Transform:ContrastCentersStretch"         , component::ContrastCentersStretch             )  
        ID_MAP_TYPE( "Transform:MinusVsAverage"                 , component::MinusVsAverage                     )  // 5
        // // ID_MAP_TYPE( "Train:Genotyping<Birdseed>"            , GenotypingTrainingPhase<algorithm::Birdseed>  )
        ID_MAP_TYPE( "Infer:Genotyping<Birdseed>"               , GenotypingCallingPhase<algorithm::Birdseed>   )
        ID_MAP_TYPE( "Infer:Genotyping<AxiomGT>"                , GenotypingCallingPhase<algorithm::AxiomGT>    )  // 7
        // ID_MAP_TYPE( "Train:BRLMMpTraining"                     , component::BRLMMpTraining                     )
        ID_MAP_TYPE( "Train:BRLMMpTrainingTentativeClustering"  , component::BRLMMpTrainingTentativeClustering  )  
        ID_MAP_TYPE( "Train:BRLMMpTrainingMinBic"               , component::BRLMMpTrainingMinBic               )  
        ID_MAP_TYPE( "Train:BRLMMpTrainingRemoveOutlier"        , component::BRLMMpTrainingRemoveOutlier        )  
        ID_MAP_TYPE( "Train:BRLMMpTrainingGrandModel"           , component::BRLMMpTrainingGrandModel           )  
        ID_MAP_TYPE( "Infer:BRLMMpGenotyping"                   , component::BRLMMpGenotyping                   )  
        ID_MAP_TYPE( "Train:BRLMMpTrainingImputation"           , component::BRLMMpTrainingImputation           )  
        ID_MAP_TYPE( "Train:BRLMMpTrainingComplete"             , component::BRLMMpTrainingComplete             )  
        ID_MAP_TYPE( "Transform:LogTransform"                   , component::birdseed::LogTransform             )
        ID_MAP_TYPE( "Train:BirdseedGrandModelProbesetChoice"   , component::birdseed::GrandModelProbesetChoice )
        ID_MAP_TYPE( "Train:BirdseedGrandModelTraining"         , component::birdseed::GrandModelTraining       )
        ID_MAP_TYPE( "Train:BirdseedProbesetTraining"           , component::birdseed::ProbesetTraining         )
        ID_MAP_TYPE( "Train:AxiomGTProbesetTraining"            , component::axiomGT::ProbesetTraining          )  // 6
        /* TODO ADD COMPONENT HERE */
        else 
        { 
            throw std::logic_error ( "the component identifier not found" );
            /* TODO another error handle */ 
        }

        return nullptr;
    }
    template<class T>
    auto operator () ( const T& component_schema )
    {
        auto name = component_schema
            .second
            .template get<std::string> ("name"); 
        return create_by_identifier ( 
              name
            , component_schema.second
        );
    }
};


#undef ID_MAP_TYPE
}
}
#else
#endif
