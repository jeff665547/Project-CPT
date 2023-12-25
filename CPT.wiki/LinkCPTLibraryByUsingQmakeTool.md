Link CPT library by using Qmake tool
===
## Pre-condition check
This section describes how to build a Qt application link with CPT library.

Before link CPT libraries, there are some preconditions need to be satisfied

1. CPT library is built and everything in project works fine.

    To check this condition you may execute any program in CPT/bin/install/bin. 
    If there's no missing link message shown, then it means the link work fine.

2. The CPT build result should put in your project and rename to CPT.
    
    We usually put the third party library in *3rdparty* directory
    
If all preconditions are satisfied, then the path tree in your project should like this: 

* intensities_dump/
    * 3rdparty/
        * CPT/
            * include/
            * lib/
            * bin/
            * share/
            * doc/
    * main.cpp
    * intensities_dump.pro
    * dialog.h
    * dialog.cpp

( assume the project named *intensities_dump* and use dialog UI widget )

## Link CPT library 

### 1. open pro file and add link command

The CPT library is developed with C\++14 standard, so we need to configure the project to use C\++14, and add CPT develop define flag.

Add the following configurations into **intensities_dump.pro**
```
QMAKE_CXXFLAGS += -DSINGLE_CPP 
CONFIG += c++14
```

In this project we will use the component in CPT that needs boost, hdf5, and opencv.

Add the following linker settings into **intensities_dump.pro**
```
unix:!macx: LIBS +=\
        $$PWD/3rdparty/CPT/lib/libboost_date_time.so\
        $$PWD/3rdparty/CPT/lib/libboost_program_options.so\
        $$PWD/3rdparty/CPT/lib/libboost_filesystem.so\
        $$PWD/3rdparty/CPT/lib/libboost_system.so\
        $$PWD/3rdparty/CPT/lib/libboost_serialization.so\
        $$PWD/3rdparty/CPT/lib/libboost_regex.so\
        $$PWD/3rdparty/CPT/lib/libboost_thread.so\
        $$PWD/3rdparty/CPT/lib/libboost_iostreams.so\
        $$PWD/3rdparty/CPT/lib/libopencv_shape.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_stitching.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_superres.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_videostab.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_viz.so.3.2.0 -lpthread\
        $$PWD/3rdparty/CPT/lib/libhdf5_cpp-shared.so.1.10.0\
        $$PWD/3rdparty/CPT/lib/libhdf5_hl-shared.so.1.10.0\
        $$PWD/3rdparty/CPT/lib/libopencv_objdetect.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_calib3d.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_features2d.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_flann.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_highgui.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_ml.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_photo.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_video.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_videoio.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_imgcodecs.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_imgproc.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_core.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libhdf5-shared.so.1.10.0
        
INCLUDEPATH += $$PWD/3rdparty/CPT/include
DEPENDPATH += $$PWD/3rdparty/CPT/include

QMAKE_RPATHDIR += $$PWD/3rdparty/CPT/lib
```
The final .pro file will looks like :
```
QT     += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = intensities_dump
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h

FORMS    += dialog.ui

unix:!macx: LIBS +=\
        $$PWD/3rdparty/CPT/lib/libboost_date_time.so\
        $$PWD/3rdparty/CPT/lib/libboost_program_options.so\
        $$PWD/3rdparty/CPT/lib/libboost_filesystem.so\
        $$PWD/3rdparty/CPT/lib/libboost_system.so\
        $$PWD/3rdparty/CPT/lib/libboost_serialization.so\
        $$PWD/3rdparty/CPT/lib/libboost_regex.so\
        $$PWD/3rdparty/CPT/lib/libboost_thread.so\
        $$PWD/3rdparty/CPT/lib/libboost_iostreams.so\
        $$PWD/3rdparty/CPT/lib/libopencv_shape.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_stitching.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_superres.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_videostab.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_viz.so.3.2.0 -lpthread\
        $$PWD/3rdparty/CPT/lib/libhdf5_cpp-shared.so.1.10.0\
        $$PWD/3rdparty/CPT/lib/libhdf5_hl-shared.so.1.10.0\
        $$PWD/3rdparty/CPT/lib/libopencv_objdetect.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_calib3d.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_features2d.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_flann.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_highgui.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_ml.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_photo.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_video.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_videoio.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_imgcodecs.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_imgproc.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libopencv_core.so.3.2.0\
        $$PWD/3rdparty/CPT/lib/libhdf5-shared.so.1.10.0

INCLUDEPATH += $$PWD/3rdparty/CPT/include
DEPENDPATH += $$PWD/3rdparty/CPT/include

QMAKE_RPATHDIR += $$PWD/3rdparty/CPT/lib
```

### 2. Make the project
```
qmake ./intensities_dump.pro && make
```

**done !**

## Online example
[intensities_dump.tar.gz](http://60.250.196.14/Data/CPT_release/intensities_dump.tar.gz) - this is the qt project without CPT library in 3rdparty directory

[CPT_binary_linux.tar.gz](http://60.250.196.14/Data/CPT_release/CPT_binary_linux.tar.gz) - this is the CPT library in linux 64bit binary build
