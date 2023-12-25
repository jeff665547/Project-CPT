### Table of Contents
1. [**Data Transformation**](#data-transformation)
    * [Contrast Centers Stretch](#contrast-centers-stretch)
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
#### * Contrast Centers Stretch: `Work in progress`
![image](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/565914bff1ce1ead3b6951b6a3101876/image.png)
![image](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/359b95155022d4f2c79f9212b7abe9ca/image.png)
![image](http://gitlab.centrilliontech.com.tw:10088/centrillion/CPT/uploads/ae4e3e4961ca3b09275ee24a7086de69/image.png)


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
                "name" : "Input:DataLoader",                            # Component DataLoader, to load CEL, CEN, CDF, CAD files
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
                "name" : "Transform:AlleleSummarization",               # Component AlleleSummarization, to summarize probes with CAD / CDF file and filter by probeset list file
                "parameter" : {
                    "probeset_list" : {                                 # The input probeset list file path
                        "type" : "ref",
                        "content" : "probeset_list"
                    }
                }
            }
          , {
                "name" : "Transform:ContrastCentersStretch",            # Component ContrastCentersStretch, to transform data range from 3 dimension to 2 dimension (-1 ~ 1)
                "parameter" : {
                    "k_scale" : {                                       # The scaling factor 'k', which change the stretch between A and B signals, default = 4
                        "type" : "literal",
                        "content" : 4
                    }
                }
            }
          , {
                "name" : "Train:BRLMMpTrainingTentativeClustering",     # Component BRLMMpTrainingTentativeClustering, to train each probeset with different method
                "parameter" : {
                    "brlmmp_type" : {                                   # The training method, optional type: Gaussian / Mahalanobis / Euclidean
                        "type" : "custom",
                        "content" : "Gaussian"
                    },
                    "k_scale" : {                                       # The number of group for clustering
                        "type" : "literal",
                        "content" : 3
                    }
                }
            }
          , {
                "name" : "Train:BRLMMpTrainingMinBic",                  # Component BRLMMpTrainingMinBic, to find the best tentative cluster for each probeset
                "parameter" : { }
            }
          , {
                "name" : "Train:BRLMMpTrainingRemoveOutlier",           # Component BRLMMpTrainingRemoveOutlier, to remove outlier sample and re-cluster for each probeset
                "parameter" : { }
            }
          , {
                "name" : "Train:BRLMMpTrainingGrandModel",              # Component BRLMMpTrainingGrandModel, to train grand model for each probeset
                "parameter" : { }
            }
          , {
                "name" : "Train:BRLMMpTrainingImputation",              # Component BRLMMpTrainingImputation, to impute miss cluster for each probeset
                "parameter" : { }
            }
          , {
                "name" : "Train:BRLMMpTrainingComplete",                # Component BRLMMpTrainingComplete, to output brlmmp training result to model file
                "parameter" : { }
            }
        ]
    }

### Genotyping Json Example
***
    {
        "context" : {                                                   # Pipeline contexts config
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
                "name" : "Transform:ContrastCentersStretch",            # Component ContrastCentersStretch, transform data range from 3D to 2D
                "parameter" : {
                    "k_scale" : {                                       # The scaling factor 'k', which change the stretch between A and B signals, default = 4
                        "type" : "literal",
                        "content" : 4
                    }
                }
            }
          , {
                "name" : "Infer:BRLMMpGenotyping",                      # Component BRLMMpGenotyping, output BRLMMp genotyping result according to input model file, default Genotyping type is Gaussian
                "parameter" : { }
            }
        ]
    }