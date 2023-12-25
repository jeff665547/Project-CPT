### Table of Contents
1. [**Data Transformation**](#data-transformation)
2. [**Training Methods**](#training-methods)
    * [Gaussian](#gaussian)
    * [Mahalanobis](#mahalanobis)
    * [Euclidean](#euclidean)
3. [**Remove Outlier**](#remove-outlier)
4. [**Grand Model Training**](#grand-model-training)
5. [**Imputation**](#imputation)
6. [**Genotyping**](#genotyping)
7. [**Training Json Example**](#training-json-example)
8. [**Genotyping Json Example**](#genotyping-json-example)

### Data Transformation
***
`Work in progress`

### Training Methods
***
#### * Gaussian: `Work in progress`
#### * Mahalanobis: `Work in progress`
#### * Euclidean: `Work in progress`

### Remove Outlier
***
`Work in progress`

### Grand Model Training
***
`Work in progress`

### Imputation
***
`Work in progress`

### Genotyping
***
`Work in progress`

### Training Json Example
***
    {
        "context" : {                                                   # Pipeline contexts config
            "output_dir" :                                              # The output path or prefix
                "OutputPath/Prefix_"
            ,
            "chip_layout" :                                             # The input file for CAD / CDF
                "ChipFile.CAD"
            ,
            "clustering_models" : [                                     # The output path of model files 
                "ModelFile.MDL"
            ],
            "quantile_norm_target_sketch" :                             # The sketch file for quantile normalization
                "TargetSketch.TXT"
            ,
            "probeset_list" :                                           # The probeset list file for allele summarization
                "ProbeSetList.PS"
            ,
            "sample_files" : [                                          # The input path of CEN / CEL files
                "Sample01.CEN"
              , "Sample02.CEN"
              , "Sample03.CEN"
            ]
        },
        "pipeline" : [                                                  # Pipeline components config
            {
                "name" : "Input:DataLoader",                            # Component DataLoader, to load CEL, CEN, CDF, CAD, or PS files
                "parameter" : { }
            }
          , {
                "name" : "Train:TargetSketchEstimation",                # Component TargetSketchEstimation, to estimate sketch for quantile normalization
                "parameter" : {
                    "target_sketch" : {                                 # The output sketch file path
                        "type" : "ref",
                        "content" : "quantile_norm_target_sketch" 
                    },
                    "scaling_factor" : {                                # The sketch scaling factor, 0 = all, default = 50000
                        "type" : "literal",
                        "content" : 0
                    }
                }
            }
          , {
                "name" : "Transform:QuantileNormalization",             # Component QuantileNormalization, do quantile normalization with sketch file
                "parameter" : {
                    "target_sketch" : {                                 # The input sketch file path
                        "type" : "ref",
                        "content" : "quantile_norm_target_sketch"
                    },
                    "scaling_factor" : {                                # The sketch scaling factor, must be the same with sketch file
                        "type" : "literal",
                        "content" : 0
                    }
                }
            }
          , {
                "name" : "Transform:AlleleSummarization",               # Component AlleleSummarization, summarize probes with CAD / CDF file and filter by probeset list file
                "parameter" : {
                    "probeset_list" : {                                 # The input probeset list file path
                        "type" : "ref",
                        "content" : "probeset_list"
                    }
                }
            }
          , {
                "name" : "Train:BirdseedGrandModelProbesetChoice",      # Component to select the grand model training probeset
                "parameter" : {
                    "rnd_nums" : {                                      # The number of probeset to be select
                        "type" : "literal",
                        "content" : 5000
                    }
                }
            }
          , {
                "name" : "Transform:LogTransform",                      # Component to logarithm the probeset signal
                "parameter" : { }
            }
          , {
                "name" : "Train:BirdseedGrandModelTraining",            # Component to train the grand model
                "parameter" : {
                    "tol" : {                                           # The tolerance of training process, means how fit of training result
                        "type" : "literal",
                        "content" : 1e-05
                    },
                    "max_iter" : {                                      # The max iteration of training
                        "type" : "literal",
                        "content" : 500
                    },
                    "n_init" : {                                        # Initial times of training process
                        "type" : "literal",
                        "content" : 20
                    },
                    "n_components" : {                                  # The component number of training model
                        "type" : "literal",
                        "content" : 3
                    }
                }
            }
          , {
                "name" : "Train:BirdseedProbesetTraining",              # Component to train the allele specific model
                "parameter" : {
                    "fitting_algo" : {                                  # The detail method used in fitting algorithm of training
                        "type" : "custom",
                        "content" : {
                            "name" : "PRFittingRT",                     # The method name of algorithm, like PRFittingRT, OCCFittingRT etc.
                            "trim_rate" : 5,                            # This parameter will used if "name" is PRFittingRT otherwise ignore, it decides the number ratio of outlier in probeset going to be trimmed.
                            "subtype" : "KMeans"                        # The subtype of method, like KMeans, GMM etc.
                        }
                    },
                    "thread_num" : {
                        "type" : "literal",
                        "content" : 32
                    }
                }
            }
        ]
    }

### Genotyping Json Example
***
    {
        "context" : {                                                   # Ppeline contexts config 
            "output_dir" :                                              # The output path or prefix
                "OutputPath/Prefix_"
            ,
            "chip_layout" :                                             # The input file for CAD / CDF
                "ChipFile.CAD"
            ,
            "clustering_models" : [                                     # The input path of model files 
                "ModelFile.MDL"
            ],
            "quantile_norm_target_sketch" :                             # The sketch file for quantile normalization
                "TargetSketch.TXT"
            ,
            "probeset_list" :                                           # The probeset list file for allele summarization
                "ProbeSetList.PS"
            ,
            "sample_files" : [                                          # The input path of CEN / CEL files
                "Sample01.CEN"
              , "Sample02.CEN"
              , "Sample03.CEN"
            ]
        },
        "pipeline" : [                                                  # Pipeline components config
            {
                "name" : "Input:DataLoader",                            # Component DataLoader, to load CEL, CEN, CDF, CAD, or PS files
                "parameter" : { }
            }
          , {
                "name" : "Train:TargetSketchEstimation",                # Component TargetSketchEstimation, to estimate sketch for quantile normalization
                "parameter" : {
                    "target_sketch" : {                                 # The output sketch file path
                        "type" : "ref",
                        "content" : "quantile_norm_target_sketch" 
                    },
                    "scaling_factor" : {                                # The sketch scaling factor, 0 = all, default = 50000
                        "type" : "literal",
                        "content" : 0
                    }
                }
            }
          , {
                "name" : "Transform:QuantileNormalization",             # Component QuantileNormalization, do quantile normalization with sketch file
                "parameter" : {
                    "target_sketch" : {                                 # The input sketch file path
                        "type" : "ref",
                        "content" : "quantile_norm_target_sketch"
                    },
                    "scaling_factor" : {                                # The sketch scaling factor, must be the same with sketch file
                        "type" : "literal",
                        "content" : 0
                    }
                }
            }
          , {
                "name" : "Transform:AlleleSummarization",               # Component AlleleSummarization, summarize probes with CAD / CDF file and filter by probeset list file
                "parameter" : {
                    "probeset_list" : {                                 # The input probeset list file path
                        "type" : "ref",
                        "content" : "probeset_list"
                    }
                }
            }
          , {
                "name" : "Infer:Genotyping<Birdseed>",                  # Component Genotyping<Birdseed>, output Birdseed genotyping result according to input model file

                "parameter" : { }
            }
        ]
    }