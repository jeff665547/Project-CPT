**The CEN file is an Affy's CEL file equivalent. It follows the basic HDF5 format.
For internal use, we have created some utilities to help the development of the array platforms in CentrillionTech.**

Please note that the executables in `bin/` with the suffix `_static`, can be used directly on linux machines. For other operating systems, you might need to recompile the code (untested yet).

### Table of Contents
1. [**Building CEN Files**](#building-cen-files)
2. [**HDF5 Schema Builder**](#hdf5_schema_builder)
3. [**Data Layout In Detail**](#data-layout-in-detail)
4. [**Read and Manipulate CEN Files**](#read-and-manipulate-cen-files)
5. [**Affy to HDF5**](#affy-to-hdf5)

### Building CEN Files
***

There are three ways to build a CEN file:

  **Method 1. From a JSON data file**

  1. First, organize all the information including all the attributes, probe intensities, and other relevant data into a JSON file, whose structured is explained in [DocumentForHDF5Builder](DocumentForHDF5Builder)
 and [`doc/hdf5_schema_details.json`](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hdf5_schema_details.json)

    See [Data layout](#data-layout-in-detail) for how to arrange intensity data in JSON in detail.
    You can also find an example of a working example: [`doc/hdf5_schema_example.json`](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hdf5_schema_example.json)

  2. Second, run the program [`hdf5_schema_builder`](#hdf5-schema-builder)
to generate the binary HDF5 file from the JSON file.
 
  3. Finally, you can [read and manuplate](#read-and-manipulate-cen-files) this CEN file.

  **Method 2. From an Affy's CEL file**

  1. You will have to first create an empty CEN template file and import all the data from a CEL file into the template.
    
    To create the empty CEN template, it is similar to the first two steps in Method 1, you create a JSON file but leave all the real data as default. Generally, you can directly use the example JSON file [`doc/hdf5_schema_example.json`](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hdf5_schema_example.json) for generating the CEN template.  

  2. Import all the data in a CEL file to this CEN template by [affy2hdf5](#affy-to-hdf5)

  3. Finally, you can [read and maniplate](#read-and-manipulate-cen-files) this CEN file.

  **Method 3. From only intensity data**

  1. You will have to first create an empty CEN template file and import all the data into the template by HDF5Viewer or HDF toolkits.
    
    To create the empty CEN template, it is similar to the first two steps in Method 1, you create a JSON file but leave all the real data as default. Generally, you can directly use the example JSON file [`doc/hdf5_schema_example.json`](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hdf5_schema_example.json) for generating the CEN template.  

  2. You prepare your intensity data into plain text and import them into the template CEN file by HDF5Viwer or HDF5 toolkits. See [read and manipulate CEN files](#read-and-manipulate-cen-files).

 
### HDF5 Schema Builder
***

This program is used to create CEN (HDF5) format file.
It needs a text template file (in JSON format) to specify the hdf5 spec.
Then run the following command

    ./hdf5_schema_builder_static <cenfile-json-schema> <cenfile-output-file>

for example:

    ./hdf5_schema_builder_static hdf5_schema_example.json example.cen

Please read the following files in detail to learn about how to write the HDF5 schema in JSON format for your own purpose.

* [hdf5_schema_quickstart.txt](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hd5_schema_quickstart.txt)
* [hdf5_schema_details.json](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hdf5_schema_details.json)


### Data Layout In Detail
***


The intensity of probes is arranged in 2D on a chip, they can be located by the 0-based row major coordinate system as follows:

| Index | col_0 | col_1 | col_2 | col_3 |
|:-:|:-:|:-:|:-:|:-:|
| row_0 | A | A | A | A |
| row_1 | B | **`B`** | B | B |
| row_2 | C | C | C | C |
| row_3 | D | D | **`D`** | D |
| row_4 | E | E | E | E |
| row_5 | F | F | F | F |
| row_6 | G | G | G | **`G`** |
| row_7 | H | H | H | **`H`** |

The letters represent the intensity, and the highlighted ones are those probes that were masked for whatever reasons. 

In the CEN file, the 2D intensity values should be concatenated into a 1D array as below. 

| A | A | A | A | B | B | B | B | C | C | C | C | D | D | D | D | E | E | E | E | F | F | F | F | G | G | G | G | H | H | H | H |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|


The masked probes are given in the form of the coordinate:

| X (row index) | Y (column index) | 
|:-:|:-:|
| 1 | 1 |
| 3 | 2 |
| 6 | 3 |
| 7 | 3 |

Please see the section below for more details.

### Read and Manipulate CEN Files
***


To open the CEN file you created in previous step, you can use HDFView official software to browse the content of a CEN file.
The HDFView browser is avalible [here](https://www.hdfgroup.org/products/java/release/download.html)

You may want to import the measurement/intensity and other attributes into a CEN file, please refer to 
[A_Guideline_for_Manipulating_CEN_File_by_using_HDFView.pptx](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/0f93874e753081a99ac28471e96bf6ee/A_Guideline_for_Manipulating_CEN_File_by_using_HDFView.pptx) in detail.


### Affy to HDF5
***


This program is used to convert an Affymatrix CEL file into CEN (HDF5) file.

The conversion only works in conditions as follows : 

1. The CEL file should be in 'Command Console Version 1' Format defined by Affymatrix, whose magic number is 59.

2. The input CEN (HDF5) file should be an uninitialised empty CEN template file, which can be generated directly by [`hdf5_schema_builder`](#hdf5_schema_builder)

   This file will be updated in place with the content from the CEL file

    ./affy2hdf5_static <empty-cenfile> <celfile>

for example:

    ./affy2hdf5_static example.cen example.cel

Then, you can use HDFView to see what content does affychip.cel have. 

All non-essential or unused fields in the CEL file will go to legacy group section in the CEN file. 

Note that CEL reader inside the affy2hdf5 binary currently support Command Console version 1 format only.