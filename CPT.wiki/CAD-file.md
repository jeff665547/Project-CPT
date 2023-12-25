**The CAD is array schema and knowledge data model file which specify the array and probe(set)s information. The utility tools are created for development the platforms in CentrillionTech. This file stands a similar function as Affy's CDF file.**

Please note that the executables in `bin/` with the suffix `_static`, can be used directly on linux machines. __For other operating systems, you might need to recompile the code (untested yet).__

### Table of Contents
1. [**Quick Start**](#quick-start)
2. [**Building CAD Files**](#building-cad-files)
3. [**Affy's CDF to CAD**](#affys-cdf-to-cad)
4. [**CadTool**](#cadtool)
5. [**CAD File in HDFView**](#cad-file-in-hdfview)

### Quick Start
***

We provide a simple way to test the functionality of `cadtool` and `cdf2cad`

### CadTool
***

Please start it by running the following command in your cadtool directory. This will output an `example.cad` from `minimal_example.json`, which contains a minimal set of options for CAD file creation and applies the default values to empty fields.

    $ ./cadtool -m json2cad -i minimal_example.json -o example.cad

Next, you can turn `example.cad` back to JSON file by running the following step for inspecting the content of `example.cad`

    $ ./cadtool -m cad2json -i example.cad -o example.json

### Affy's CDF to CAD
***

Please run the following instruction to test `cdf2cad`

    $ ./cdf2cad -i example.cdf -o example.cad -p example.probe_tab -a example.annot.csv  


### Building CAD Files
***

There are two ways to build a CAD file:

  **Method 1. From a JSON data file**
  
  1. First, organize all information including all the attributes, array column, row numbers, probeset id range, probe sequence information, and other relevant data into a JSON file, see [**CAD format spec**](CAD-format-spec) for more details and examples

  2. Second, run the program [`cadtool`](#cadtool) in __json2cad mode__ and the CAD file will be generate in few seconds.
  
  **Method 2. From Affy's library files**

  1. There's three library files are required: CDF file(_.cdf_), probe table(_.probe_tab_), annotation file(_.annot.csv_), please make sure all three files are ready.
  
  2. run the program [`cdf2cad`](#affys-cdf-to-cad), and the CAD file fill be generate in few seconds.

### Cadtool
***

CAD file is a binary file created by HDF5_Group library, to modify and view the data content in the file, a human readable text format is needed. For this reason the JSON and CAD format 2-way mapping is designed. [Here](CAD-format-spec) is the CAD (also JSON) format spec in detail.

To convert the CAD to JSON just run the program `cadtool` in cad2json mode, for example: 
    
    cadtool -m cad2json -i in.cad -o out.json

The __-m__ specify the mode, __-i__ specify the input CAD file, and __-o__ specify the output JSON file.

The full usage is : 

    Allowed options:
      -h [ --help ]         show help message
      -i [ --input ] arg    input file
      -o [ --output ] arg   output file
      -m [ --mode ] arg     mode (json2cad/cad2json)

### Affy's CDF to CAD
***

This program is used to convert Affy's library files into CAD file.

The conversion only works in condition as follows : 

1. The library files is from Affy's Axiom platform 

2. At least one CDF file(_.cdf_), probe table(_.probe_tab_), annotation file(_.annot.csv_) are necessary.

To convert the library file into CAD just run the program `cdf2cad` directly, for example : 

    cdf2cad_static -i Axiom_GW_Hu-CHB_SNP.r4.cdf -o xxx.cad -p Axiom_GW_Hu-CHB_SNP.probe_tab -a Axiom_GW_Hu-CHB_SNP.na34.annot.csv

The __-i__ specify the input cdf file, __-p__ is the probe_tab file and __-a__ is annot.csv file.

The full usage is  : 

    Allowed options:
      -h [ --help ]          show help message
      -i [ --input ] arg     input cdf file
      -o [ --output ] arg    output cad file
      -p [ --probe_tab ] arg probe_tab file
      -a [ --annot_csv ] arg annot_csv file

Note : 
*There is a known excessive memory footprint issue in the third party libraries in Windows systems. We found this program takes up more than 4G of ram in Windows whereas it takes only 1.4 G ram in Linux when converting a big CDF file.*

### CAD File in HDFView
***

#### 1. Group and dataset in HDFView
![GroupDataset](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/56423686fcdd8d21dec443bae2cfdd91/GroupDataset.PNG)

#### 2. Attribute in HDFView
![Attribute](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/fb741a10f1a545e05b784659d40554b0/Attribute.PNG)

#### 3. Tables in HDFView
![Tables](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/035950e423a28900f232129d4061ee3d/Tables.PNG)

#### 4. Probeset_list in HDFView
![ProbesetList](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/bdeff5b621edccdd314c7faa2935dbc8/ProbesetList.PNG)

#### 5. Probe_list in HDFView
![ProbeList](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/dac2705db263749453e817cd559f9d5e/ProbeList.PNG)

#### 6. Cross reference between probeset_list and probe_list in HDFView
![CrossRefer](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/36dbce4bc42d8e3e5d05875d82ca36fd/CrossRefer.PNG)

