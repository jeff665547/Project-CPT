Project Release Workflow and Description
===

## Prepare

Before everything starts, we should have a **requirement list** which probably includes following items:

1. Release modules
2. Release document
3. Release source code
4. Release platform(s)
5. IDE or SDK support ( if source code release is needed )

This list may send to you by email or on the meeting, please make sure everything is well-defined.

## Release Item Check

In this step, we need to make sure every item on the list is ready and the requirement is satisfied.

### Release Module ( Binary Targets )

* CMake Script
Make sure the targets are in the CMake configure file. In CPT project, all release targets' CMake build scripts are put in ```CPT/lib/cmake/app```

* Compile Flag
Make sure every binary object is built in release option ( -O2 or -O3 and no -g ).

### Release Document 

There are two main ways we publish our document. 
The requirement list should identify which and what type of document should be prepared. 

* gitlab WIKI
Check the document state on [gitlab](
/)
[CPT wiki](home)

* Doxygen
All doxygen generate result can be found at ```CPT/doc/doxygen``` and doxygen configure file is at ```CPT/doc.cfg```.

The online doxygen site is at [doxygen](http://cptdoc.centrilliontech.com.tw). 

Please upload the generated result to there.

### Unit test

Create a **unit test list** record the related unit tests of this release.

## Build Source Code

In general, we just need to build the project with classic build process and the build result will be generated in ```CPT/bin/install```, but in this section, there's two main subject need to be discussed **platform** and **source code release**.

### Platform

To make sure the linked library version is what we need and no libraries are missing in the project, the project should be built on a clean platform and only has the libraries listed in the project's *pre-requirement* or *pre-install* list. 

For CPT on Linux the pre-installation are:
    
   * python2.7
   * python3.5
   * g++ 6.0 or later
   * CMake 3.2 or later
   * OpenSSL

For CPT on Windows the pre-installation are:
    
   * python2.7
   * python3.5
   * MinGW with g++ 6.0 or later
   * CMake 3.2 or later
   
### Source Code Release

Because the main source codes of Centrillion projects are all headers, so the source code release task is roughly equal to release all headers (\*.hpp and \*.h) otherwise, only some necessary headers(\.h) are released.

### CMake

To achieve the goal described below, we provide some option in project CMake script.
The command and argument of CMake usage can be found at [here](CPTCMakeUsage)

## Check Build Result

There are 4 subjects need to be checked

* Requirment match
* Binary linkage
* Program behavior
* IDE, SDK support 

### Requirment Match

Check the build result under ```CPT/bin/install``` is match the requirement list.

### Binary linkage

For Linux release, our library use "relative rpath" to link the library, this design is used to prevent multiple versions of the same library found in the target system and force all released binary link the library provided by the project.

The following steps are used to make sure the library linkages are in our expect.

1. Execute every target and make sure no "missing link" or similar error message.
2. Link or import the library file( .py,.so,.a,.dll ) released by project and make sure no “missing link” or similar error.
3. If platform is Linux check the link path with the following command
```
ldd -r [dynamic binary (executable or dynamic library)]
```
This command will print the library path of a binary file, and you should check every library are linked to correct place. 
For project provided library, it should be a relative path point to ```../lib/```.


### Program Behavior

* Unit test 
If the target has some related unit tests then just check the unit test result by input ``` make test ``` or execute the unit test binary executable.

* Manual test
For the target has no unit test on the **unit test list** ( create at [Release Item Check - Unit test](unit-test)), we have to manual test them.
For executable, Manually execute the target and check the result.
For library, Manually link or import the target and check the result.

### IDE, SDK support

* CMake support
Follow the step at [here](LinkCPTLibraryByUsingCMakeTool) and make sure the library can import correctly.

* QT support
Follow the step at [here](LinkCPTLibraryByUsingQmakeTool) and make sure the library can import correctly.