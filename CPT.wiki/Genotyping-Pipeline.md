### Table of Contents
1. [**Pipeline Usage**](#pipeline-usage)
2. [**Pipeline Frameworks**](#pipeline-frameworks)
3. [**Component List**](#component-list)
4. [**Parameter Table**](#parameter-table)
5. [**Json Example**](#json-example)

### Pipeline Quick Start

- If your CPT toolkit has not installed yet, please refer to [Installation](Download-package)
- Assume that download and working directory is `/root/wei_test/`.
- Download CDF file: http://60.250.196.14/Data/CPT_release/genotyping_data/Axiom_GW_Hu-CHB_SNP_CDF.tar.gz to `/root/wei_test` (**Axiom_GW_Hu-CHB_SNP.r2.cdf** and **biallelic.ps**)
- Download CEL files http://60.250.196.14/Data/CPT_release/genotyping_data/GSE78098_CEL.tar.gz to `/root/wei_test/` (**raw_data/*.cel**)
- Extract **example.7z** to `/root/wei_test/CPT_release/` (**example/***)
- Open file `/root/wei_test/CPT_release/` (**example/birdseed_training_example.json**) and modify it as below

:fire:`Be aware of some path labels below`
```json
{ 
    "context" : 
    {
        "output_dir" : "/root/wei_test/",
        "clustering_models": [
            "/root/wei_test/test.mdl"
        ],
        "chip_layout": "/root/wei_test/Axiom_GW_Hu-CHB_SNP.r2.cdf", //=>.cad
        "sample_files" : [
            "/root/wei_test/raw_data/GSM2066668_206-001_CHB.CEL", //=>.cen
            "/root/wei_test/raw_data/GSM2066669_206-003_CHB.CEL",
            ....
        ]
        ...
    },
    "pipeline": [
        ...
                "target_sketch_prefix" : {
                    "type" : "literal",
                    "content" : "/root/wei_test/tse_",
                    "comment" : "input"
                },
        ...
        {
            "name": "Transform:AlleleSummarization",
            "parameter": {
                "probeset_list" : {
                    "type" : "literal",
                    "content" : "/root/wei_test/biallelic.ps",
                    "comment" : "input"
                }
            }
        },
    ]
    ...
}
```

```txt
$/root/wei_test/CPT_release/install/bin/pipeline_builder -i /root/wei_test/CPT_release/example/birdseed_training_example.json -o output.json
```
 
### Pipeline Usage
***

Options via `./pipeline_builder -h`

    Allowed options:
      -h [ --help ]         show help message
      -i [ --input ] arg    input json file
      -o [ --output ] arg   output json file

Main usage:

    ./pipeline_builder -i input.json
    ./pipeline_builder -i input.json -o output.json

There are two step to use pipeline builder.

First to create json file refer to [Json Example](#json-example) as input file and run as follow `./pipeline_builder -i input.json -o output.json`.

if `--output` option is not specified, the output json will print out to screen.

Input json contain a list of components, the execution order of processing unit follows the order of the components you listed in the input json file.

Check [Pipeline Frameworks](#pipeline-frameworks) for pipeline detail.

Check [Component List](#component-list) and [Parameter Table](#parameter-table) for components detail.

Check [Json Example](#json-example) for json detail.


### Pipeline Frameworks
***

                      | Component and Parameter |
    Input Json file   | Component and Parameter |
                      | ... ... ... ... ... ... |
           V
           V
           V          | V Setup DataPool
           V          | V Config Components
           V          | V Run pipeline with each component
                      |   |
    Pipeline Builder  |   | V Component Initializaion, read stuff from datapool
                      |   | V Component Start
           V          |   | V Component End, write stuff into datapool
           V          |
           V          | > End of pipeline
           V

    Output Json file or stdcout on screen

### Component List
***

This session list all usable components. Those components can be assembled and swapped easily by creating the json input file. For more example, please visit [**Json Example**](#json-example)

    {
        "name" : "Input:DataLoader",                            # Component to load CEL, CEN, CDF, CAD files
        "name" : "Train:TargetSketchEstimation",                # Component to estimate sketch for quantile normalization
        "name" : "Transform:QuantileNormalization",             # Component do quantile normalization with sketch file
        "name" : "Transform:AlleleSummarization",               # Component to summarize probes with CAD / CDF file and filter by probeset list file
        "name" : "Transform:ContrastCentersStretch",            # Component to transform data range from 3 dimension to 2 dimension (-1 ~ 1)
        "name" : "Train:BRLMMpTrainingTentativeClustering",     # Component to train each probeset with different method
        "name" : "Train:BRLMMpTrainingMinBic",                  # Component to find the best tentative cluster for each probeset
        "name" : "Train:BRLMMpTrainingRemoveOutlier",           # Component to remove outlier sample and re-cluster for each probeset
        "name" : "Train:BRLMMpTrainingImputation",              # Component to impute miss cluster for each probeset
        "name" : "Train:BRLMMpTrainingGrandModel",              # Component to train grand model for each probeset
        "name" : "Train:BRLMMpTrainingComplete",                # Component to output brlmmp training result to model file
        "name" : "Infer:BRLMMpGenotyping",                      # Component to output brlmmp genotyping result according to input model file, default Genotyping type is Gaussian
        "name" : "Train:BirdseedGrandModelProbesetChoice",      # Component to select the grand model training probeset
        "name" : "Transform:LogTransform",                      # Component to logarithm the probeset signal
        "name" : "Train:BirdseedGrandModelTraining",            # Component to train the grand model
        "name" : "Train:BirdseedProbesetTraining",              # Component to train the allele specific model
        "name" : "Infer:Genotyping<Birdseed>"                   # Component to output Birdseed genotyping result according to input model file
    }

### Parameter Table
***

This session lists all available components with their parameters. The parameter defined by type and content, see below for detail description. For more examples, reference to [**Json Example**](#json-example)

    {
        "Parameter" :
        {
            "type" :
            {
                "literal" : "input tpye, can be words or numbers",
                "ref"     : "reference type, it reference to the same tag within json.context",
                "custom"  : "user custom type"
            },

            "content" : "set content for parameter"
        }
    }
 
__* Input:DataLoader__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| thread_num | literal | 16 | The number of threads for multicore process |
| is_tsv | literal | false | The component tsv output |


__* Train:TargetSketchEstimation__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| target_sketch | ref | quantile_norm_target_sketch | The path of sketch file, default reference to tag "quantile_norm_target_sketch" in json input context |
| scaling_factor | literal | 50000 | The sketch scaling factor, 0 = all |
| thread_num | literal | 16 | The number of threads for multicore process |


__* Transform:QuantileNormalization__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| target_sketch | ref | quantile_norm_target_sketch | The path of sketch file, default reference to tag "quantile_norm_target_sketch" in json input context |
| scaling_factor | literal | 50000 | The sketch scaling factor, 0 = all |
| thread_num | literal | 16 | The number of threads for multicore process |
| is_tsv | literal | false | The component tsv output |


__* Transform:AlleleSummarization__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| probeset_list | ref | probeset_list | The input probeset list file path, default reference to tag "probeset_list" in json input context |
| thread_num | literal | 16 | The number of threads for multicore process |
| is_tsv | literal | false | The component tsv output |


__* Transform:ContrastCentersStretch__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| k_scale | literal | 4 | The set scaling factor 'k', which change the stretch between A and B signals |
| thread_num | literal | 16 | The number of threads for multicore process |
| is_tsv | literal | false | The component tsv output |


__* Train:BRLMMpTrainingTentativeClustering__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| brlmmp_type | custom | Euclidean | The training method, optional type: Gaussian / Mahalanobis / Euclidean |
| k_cluster | literal | 3 | The number of group for clustering |
| thread_num | literal | 16 | The number of threads for multicore process |


__* Train:BRLMMpTrainingMinBic__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| thread_num | literal | 16 | The number of threads for multicore process |


__* Train:BRLMMpTrainingRemoveOutlier__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| thread_num | literal | 16 | The number of threads for multicore process |


__* Train:BRLMMpTrainingImputation__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| N/A | N/A  | N/A | N/A |


__* Train:BRLMMpTrainingGrandModel__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| N/A | N/A  | N/A | N/A |


__* Train:BRLMMpTrainingComplete__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| N/A | N/A  | N/A | N/A |


__* Infer:BRLMMpGenotyping__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| brlmmp_type | custom | Gaussian | The training method, optional type: Gaussian / Mahalanobis / Euclidean |
| cut_off | literal | 0.66 | The cut_off, successful calling defined as confidence > cut_off, confidence = 1 - P |
| output_path | literal | The output_dir/genotype.tsv | output path |
| thread_num | literal | 16 | The number of threads for multicore process |


__* Train:BirdseedGrandModelProbesetChoice__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| rnd_nums | literal | 5000 | The number of probeset to be select |
| rnd_probeset_ids | ref | GrandModelProbesetIds | The output select ids |
| probeset_cube | ref | probeset_cube | The cube of all probesets |


__* Transform:LogTransform__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| probeset_ids | ref | GrandModelProbesetIds | A set of selected probeset ids |
| log_allele_min | ref | LogAlleleMin | The output min allele signal of selected probeset  |
| log_allele_max | ref | LogAlleleMax | The output max allele signal of selected probeset |
| log_allele_avg | ref | LogAlleleAvg | The output average allele signal of selected probeset |
| probeset_signal | ref | LogProbesetSignal | The output logarithm probeset signals |
| probeset_cube | ref | probeset_cube | The cube of all probesets |


__* Train:BirdseedGrandModelTraining__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| log_allele_min | ref | LogAlleleMin | The min allele signal of selected probeset  |
| log_allele_max | ref | LogAlleleMax | The max allele signal of selected probeset |
| log_allele_avg | ref | LogAlleleAvg | The average allele signal of selected probeset |
| log_probeset_signal | ref | LogProbesetSignal | The logarithm probeset signal |
| n_components | literal | 3 | The component number of training model |
| n_init | literal | 20 | Initial times of training process |
| tol | literal | 1e-5 | The tolerance of training process, means how fit of training result |
| max_iter | literal | 500 | The max iteration of training |


__* Train:BirdseedProbesetTraining__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|
| fitting_algo | custom | N/A | The detail method used in fitting algorithm of training |
| fitting_algo.content.name | N/A | N/A | The method name of algorithm, like PRFittingRT, OCCFittingRT etc. |
| fitting_algo.content.subtype | N/A | N/A | The subtype of method, like KMeans, GMM etc. |
| fitting_algo.content.subtype_metric | N/A | N/A | This parameter will used if subtype is KMeans otherwise ignore, it decides the KMeans distance scoring function, can be EuclideanDistance, or MahalanobisDistance etc. |
| fitting_algo.content.trim_rate | N/A | N/A | This parameter will used if "name" is PRFittingRT otherwise ignore, it decides the number ratio of outlier in probeset going to be trimmed. |
| fitting_algo.content.threshold_rate| N/A | N/A | This parameter will used if "name" is OCCFittingRT otherwise ignore, it decides the outlier component threshold of model. |
| fitting_algo.content.min_score | N/A | N/A | This parameter will used if "name" is AbsPFittingRT otherwise ignore, it define an absolute probability as a outlier minimum score. |
| thread_num | literal | 16 | The number of threads for multicore process |


__* Infer:Genotyping<Birdseed>__

| parameter | default type | default content | Description |
|:---------:|:----:|:---------------:|:-----------:|:-----------:|
| probeset_cube | ref  | probeset_cube | The input probeset allele signal |
| summary_of_probesets | ref  | SummaryOfProbesets | The call rate of samples |
| results | ref  | Genotypes | The genotype result table |

### Json Example
***

This session is a json format example.

If you are looking for usable json template, please look at following content: 
* [BRLMMP Training Json](BRLMMp#training-json-example)
* [BRLMMP Genotyping Json](BRLMMp#genotyping-json-example)
* [Birdseed Training Json](Birdseed#training-json-example)
* [Birdseed Genotyping Json](BirdSeed#genotyping-json-example)

***

    {
        "context" : {                           # Pipeline contexts config
            "output_dir" :                      # The output path or prefix
                "OutputPath/Prefix_"
            ,
            "chip_layout" :                     # The input file for CAD / CDF
                "ChipFile.CAD"
            ,
            "clustering_models" : [             # The output or input path of model files
                "ModelFile.MDL"
            ],
            "quantile_norm_target_sketch" :     # The sketch file for quantile normalization
                "TargetSketch.TXT"
            ,
            "probeset_list" :                   # The probeset list file for allele summarization
                "ProbeSetList.PS"
            ,
            "sample_files" : [                  # The input path of CEN / CEL files
                "Sample01.CEN"
              , "Sample02.CEN"
              , "Sample03.CEN"
            ]
        },
        "pipeline" : [                          # Pipeline components config
            {
                "name" : "Input:DataLoader",    # Component 'name'
                "parameter" : {                 # Parameters
                    "thread_num" : {            # The parameter 'thread_num'
                        "type" : "literal",     # The 'type' for parameter 'thread_num', optional types: literal / ref / custom
                        "content" : 16          # The 'content' for parameter 'thread_num'
                    }
                }
            },
            { ... },                            # Other Component
            { ... },                            # Other Component
            { ... },                            # Other Component
        ]
    }