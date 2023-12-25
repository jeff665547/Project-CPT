macro( set_if_not_defined var_name var_val)
    if ( NOT DEFINED ${var_name} )
        set ( ${var_name} ${var_val} )
    endif()
endmacro()
macro( show_var n )
    message( STATUS "${n} : ${${n}}" )
endmacro()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()
get_filename_component( CPT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../ ABSOLUTE )
show_var ( CPT_ROOT )
set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CPT_ROOT} )

# boost
set( Boost_USE_STATIC_LIBS    OFF )
set( Boost_USE_MULTITHREADED  ON  )
set( Boost_USE_STATIC_RUNTIME OFF )
find_package( Boost REQUIRED COMPONENTS 
    date_time 
    program_options 
    filesystem 
    system 
    serialization 
    regex 
    thread 
    iostreams 
)
set ( BOOST_COMMON_LIBS
    ${Boost_DATE_TIME_LIBRARY_RELEASE} 
    ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE} 
    ${Boost_FILESYSTEM_LIBRARY_RELEASE} 
    ${Boost_SYSTEM_LIBRARY_RELEASE} 
    ${Boost_SERIALIZATION_LIBRARY_RELEASE} 
    ${Boost_REGEX_LIBRARY_RELEASE} 
    ${Boost_THREAD_LIBRARY_RELEASE} 
    ${Boost_IOSTREAMS_LIBRARY_RELEASE} 
)
show_var( Boost_LIBRARY_DIRS )
show_var( Boost_INCLUDE_DIRS )

# gtest
set_if_not_defined( GTEST_ROOT ${CPT_ROOT} )
find_package( GTest REQUIRED )
show_var( GTEST_INCLUDE_DIRS )
show_var( GTEST_LIBRARIES )
show_var( GTEST_BOTH_LIBRARIES )
show_var( GTEST_MAIN_LIBRARIES )

# hdf5
if ( EXISTS "${CPT_ROOT}/share/cmake/FindHDF5.cmake" )
    set_if_not_defined( HDF5_DIR ${CPT_ROOT}/share/cmake )
    find_package (HDF5 REQUIRED NAMES hdf5 COMPONENTS C CXX HL shared)
    set( HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS} )
    set( HDF5_LIBRARIES ${HDF5_C_SHARED_LIBRARY} ${HDF5_CXX_SHARED_LIBRARY} ${HDF5_HL_SHARED_LIBRARY} )
else()
    find_package( HDF5 REQUIRED COMPONENTS C CXX HL)
endif()
show_var( HDF5_FOUND )
show_var( HDF5_INCLUDE_DIRS )
show_var( HDF5_LIBRARIES )

# opencv
if ( EXISTS "${CPT_ROOT}/share/OpenCV/OpenCVConfig.cmake" )
    set_if_not_defined( OpenCV_DIR ${CPT_ROOT}/share/OpenCV )
endif()
show_var( OpenCV_DIR )
set( OpenCV_SHARED      OFF )
set( OpenCV_STATIC      ON  )
find_package( OpenCV CONFIG REQUIRED )
show_var( OpenCV_LIBS )
show_var( OpenCV_INCLUDE_DIRS )

# curl
find_package( CURL REQUIRED )
show_var( CURL_LIBRARY_DIRS )
show_var( CURL_INCLUDE_DIRS )
show_var( CURL_LIBRARIES    )

# armadillo 
find_package( Armadillo REQUIRED )
show_var( ARMADILLO_INCLUDE_DIRS )
show_var( ARMADILLO_LIBRARIES )

# python2.7
set_if_not_defined( PYTHON2_ROOT ${CPT_ROOT} )
show_var( PYTHON2_ROOT )
find_path( PYTHON2_INCLUDE_DIRS 
    NAMES python2.7 
    PATHS ${PYTHON2_ROOT}/include
)
find_library( PYTHON2_LIBRARIES 
    NAMES python2.7 
    PATHS ${PYTHON2_ROOT}
)
get_filename_component( PYTHON2_LIBRARIES_DIR
    ${PYTHON2_LIBRARIES}
    DIRECTORY 
)
set( PYTHON2_INCLUDE_DIRS ${PYTHON2_INCLUDE_DIRS}/python2.7 )
show_var( PYTHON2_INCLUDE_DIRS )
show_var( PYTHON2_LIBRARIES )
show_var( PYTHON2_LIBRARIES_DIR )


# python3.5
set_if_not_defined( PYTHON3_ROOT ${CPT_ROOT} )
show_var( PYTHON3_ROOT )
find_path( PYTHON3_INCLUDE_DIRS 
    NAMES python3.5/pyconfig.h 
    PATHS ${PYTHON3_ROOT}/include
)
find_library( PYTHON3_LIBRARIES 
    NAMES python3.5m 
    PATHS ${PYTHON3_ROOT}
)
get_filename_component( PYTHON3_LIBRARIES_DIR
    ${PYTHON3_LIBRARIES}
    DIRECTORY 
)
set( PYTHON3_INCLUDE_DIRS ${PYTHON3_INCLUDE_DIRS}/python3.5 )
show_var( PYTHON3_INCLUDE_DIRS )
show_var( PYTHON3_LIBRARIES )
show_var( PYTHON3_LIBRARIES_DIR )


# mlpack
set_if_not_defined( MLPACK_ROOT ${CPT_ROOT} )
set( MLPACK_DIR ${CMAKE_CURRENT_LIST_DIR} )
find_package( MLPACK CONFIG )
get_filename_component( MLPACK_LIBRARY_DIRS
    ${MLPACK_LIBRARIES}
    DIRECTORY 
)
if( MLPACK_INCLUDE_DIRS AND MLPACK_LIBRARIES )
    set(MLPACK_FOUND ON)
else()
    message( STATUS "mlpack not found" )
    set(MLPACK_FOUND OFF)
endif()
show_var( MLPACK_INCLUDE_DIRS )
show_var( MLPACK_LIBRARY_DIRS )
show_var( MLPACK_LIBRARIES )


# CPT 
set ( CPT_INCLUDE_DIRS ${CPT_ROOT}/include )
set ( CPT_DEPENDENT_LIBS 
    ${BOOST_COMMON_LIBS}
    ${HDF5_LIBRARIES}
    ${OpenCV_LIBS}
    ${CURL_LIBRARIES}
    ${ARMADILLO_LIBRARIES}
    ${PYTHON2_LIBRARIES}
    ${PYTHON3_LIBRARIES}
    ${MLPACK_LIBRARIES}
)
show_var( CPT_INCLUDE_DIRS )
show_var( CPT_DEPENDENT_LIBS )
# TODO CPT_FOUND
