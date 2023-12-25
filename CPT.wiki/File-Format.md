**This session descripted the file formats for CPT use.
CAD and CEN files are both binary file base on the open source project _HDF5_ from [_The HDF Group_](https://www.hdfgroup.org/why-hdf/), and the HDF5 format is designed for portable using and large scaling data.
CAD file is a central core to use CPT tools, and it descripted the details about the chip info which allow us to summarize probesets with probes. 
CEN file as an input file of our CPT tools, it records probe intensities from chip image analysis, and also contents raw image file for the image analysis use.**

### Table of Contents
1. [**CAD File**](CAD-file)
2. [**CEN file**](CEN-file) 

##### Note. All the genome regions start with `0-base`.

### CAD File
***
The CAD is a file format of chip schema specified for the chip layout. It contents the information of probes and probesets which are designed based on the HDF5 from [__The HDF Group__](https://www.hdfgroup.org/why-hdf/).
Here we have a series of utility tools for creating the CDF file. The detail descriptions of CAD file and the usage of utility tools are specifying in the [__CAD File__](CAD-file) and [__CAD Format Spec__](CAD-format-spec).

### CEN File
***
The CEN is a file format recorded chip data, and it contents probe intensity and the raw image of chip.
It is a binary file designed base on the HDF5 from [__The HDF Group__](https://www.hdfgroup.org/why-hdf/).
Here we have a series of utility tools for creating the CEN file. The detail descriptions of CEN file and the usage of tools are specifying in the [__CEN File__](CEN-file), [__Quick Start of HDF5 Schema__](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hd5_schema_quickstart.txt), [__Details of HDF5 Schema__](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/blob/master/doc/hdf5_schema_details.json), [__Document For HDF5 Builder__](#DocumentForHDF5Builder) and [__A Guideline for Manipulating CEN File by using HDFView__](http://gitlab.centrilliontech.com.tw:10080/centrillion/CPT/uploads/0f93874e753081a99ac28471e96bf6ee/A_Guideline_for_Manipulating_CEN_File_by_using_HDFView.pptx).