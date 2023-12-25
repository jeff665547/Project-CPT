CPT Library Structure
===

## Intro

This section will describe the library structure and design, we suggest you should read carefully if you are a library/downstream application developer.

The CPT is a big C++ pure template library, which means every function of CPT can be called by include header file only.

Although CPT is pure header library, it still needs binary library link from upstream dependent libraries.

Here is a list of CPT upstream dependencies : 

* OpenCV - 3.2.0
* Boost - 1.63.0
* GTest - 1.7.0
* Armadillo - 7.600.2
* MLpack - 2.1.1
* HDF5 - 1.10.0
* Python - 2.7/3.5 ( both )

To use CPT library correctly, these libraries should be linked to your developed module.

To resolve these massive dependencies, CPT carries these dependencies inside the project, with binary pre-build and online download and build script.

## Source Code Structure
Here is the CPT source code tree you may need to concern.

* CPT/
    * include/
    * src/
        * application/
    * unit_test/
    * lib/
    * env/

Basically, all libraries source codes are in include directory, but there still some .cpp files located in src and unit_test.

### src 
In src directory, most .cpp files in this directory are used to build the executable binary, which can be ignored by a library user or application developer, 
others are directly wrapped of these executable( application ) which can be ignored, too.

### unit_test
All files in this directory are for CPT libraries' module test, again it can be ignored by a library user or application developer.

### lib 
This directory place some third party source/binary files which will be used in compile and linking times.

### env 
This directory contains **the complete build of all upstream dependencies** for 2 platforms  Windows x64 MinGW and Linux x64 GNU builds.

## Build Result Structure
The built process not only can build the application and unit_test of CPT but also upstream dependencies. By default, the build process will use the pre-build binaries in lib/ and env/ under source code directory and all result will generate in bin/install under source code directory. 

There's also other options for the build process, the full usage of CMake program in CPT can be found [HERE](CPTCMakeUsage).

**For downstream develop, please import these build results to your awesome project.**

The build result tree is shown as follow : 

* bin/install/
    * include/ 
    * lib/
    * bin/
    * share/
    * doc/

### include 
This directory contains a copy of CPT headers and related upstream dependencies' ( which described [here](#intro) ) develop header.

### lib
This directory contains the binary shared library ( Linux ) and the static library ( Linux and Windows ) of CPT upstream dependencies ( [here](#intro) ) and several applications wrapped libraries of CPT.

### bin
This directory place the executable and windows shared libraries ( .dll ) file.

### share 
This directory contains several CPT upstream dependent libraries CMake configure modules. 

These CMake modules are useful if you are using CMake build system.

### doc
The API reference and document of CPT library.
