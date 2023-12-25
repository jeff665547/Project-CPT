Link CPT library by using CMake tool
===
## Pre-condition check

This section describes how to use CMake tool build your project and link with CPT library.

Before link CPT libraries, there are some preconditions need to be satisfied

1. CPT library is built and everything in project works fine.

    To check this condition you may execute any program in CPT/bin/install/bin. 
    If there's no missing link message shown, then it means the link work fine.

2. The CPT build result should put in your project and rename to CPT.
    
    We usually put the third party library in *lib* directory
    
If all preconditions are satisfied, then the path tree in your project should like this: 

* intensities_dump_cmake/
    * lib/
        * CPT/
            * include/
            * lib/
            * bin/
            * share/
            * doc/
    * src/

( assume the project named *intensities_dump_cmake* and is a command line project)

## Link to CPT library ( with a minimum example ) 

### 1. Create the main code

For testing, we use the CPT built-in application **intensities_dump** as an example program.

Create a main.cpp under src directory and put the following code into main.cpp

```clike=
#include <CPT/application/intensities_dump/main.hpp>
int main( int argc, const char* argv[] )
{
    cpt::application::intensities_dump::OptionParser option_parser(argc, argv);
    auto&& intensities_dumper (
        cpt::application::intensities_dump::make( option_parser )
    );
    intensities_dumper();
    return 0;
}
```

### 2. Create the CMakeLists.txt

First, we add some general CMake commands which contain project name setting, CMake version checks etc.

```clike
cmake_minimum_required (VERSION 3.2) # CMake version check
project (CPT_MY_APP) # project name
set( CMAKE_VERBOSE_MAKEFILE ON ) # let makefile tells more information
```

### 3. Let CMake find CPT library

The CPT library provides an awesome CMake configure file for developers to find the massive upstream dependencies, all you need to do is call the configure file.

To call the configure file, you need to set the ```CPT_DIR``` variable and use CMake built-in function ```find_package``` to invoke the CPT configure file.

The CPT_DIR is the path to ```CPTConfig.cmake``` file, which is placed in ```CPT/share```.

```cmake
set( CPT_DIR "${CMAKE_SOURCE_DIR}/lib/CPT/share" )
find_package(CPT CONFIG)
```

The CPT find package module will set these main variables:
```
CPT_DEPENDENT_LIBS
CPT_INCLUDE_DIRS
```
And other upstream dependent libraries' CMake configuration variable, all of them will be shown at CMake configure time.

### 4. Project build configuration
Now we use the variables set by find_package to link our project.
```
add_executable( my_intensities_dump ${CMAKE_SOURCE_DIR}/src/main.cpp ) # your executable 
target_include_directories( my_intensities_dump PUBLIC ${CPT_INCLUDE_DIRS} ) # add include path
target_link_libraries( my_intensities_dump ${CPT_DEPENDENT_LIBS} ) # link the libraries
target_compile_definitions( my_intensities_dump PRIVATE SINGLE_CPP ) # set the compile flag for CPT ""this is necessary""
target_compile_options( my_intensities_dump PRIVATE -std=c++14 -O3) # use c++14 standard and optimize the performance
```
The code below will make the build process generate the executable binary named ```my_intensities_dump```
### 5. Configure and Build
```
mkdir build
cd build
cmake .. && make
```

**done !**

## Online example
[intensities_dump_cmake.tar.gz](http://60.250.196.14/Data/CPT_release/intensities_dump_cmake.tar.gz) - This is the CMake project without CPT library in lib directory

[CPT_binary_linux.tar.gz](http://60.250.196.14/Data/CPT_release/CPT_binary_linux.tar.gz) - This is the CPT library in linux 64bit binary build