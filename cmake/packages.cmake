if( MSVC )
    set(GTest_CMAKE_ARGS CMAKE_ARGS 
        CMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
    )
endif()
if( MINGW )
    set(ZLIB_BUILD_SHARED_LIBS OFF)
    set(BZip2_BUILD_SHARED_LIBS OFF)
    set(Boost_BUILD_SHARED_LIBS OFF)
    set(OpenCV_ENABLE_PRECOMPILED_HEADERS OFF)
    set(CURL_USE_WINSSL ON)
    set(CURL_USE_OPENSSL OFF)
    set(OpenBLAS_VERSION 0.3.3)
else()
    set(ZLIB_BUILD_SHARED_LIBS ON)
    set(BZip2_BUILD_SHARED_LIBS ON)
    set(Boost_BUILD_SHARED_LIBS ON)
    set(OpenCV_ENABLE_PRECOMPILED_HEADERS ON)
    set(CURL_USE_WINSSL OFF)
    set(CURL_USE_OPENSSL ON)
    set(OpenBLAS_VERSION 0.3.0-p2)
endif()

hunter_config(Screw 
    VERSION ${HUNTER_Screw_VERSION}
)
hunter_config(
    ZLIB 
    VERSION ${HUNTER_ZLIB_VERSION}
    CMAKE_ARGS 
        BUILD_SHARED_LIBS=${ZLIB_BUILD_SHARED_LIBS}
)
hunter_config( BZip2 
    VERSION ${HUNTER_BZip2_VERSION}
    CMAKE_ARGS
        BUILD_SHARED_LIBS:BOOL=${BZip2_BUILD_SHARED_LIBS}
)

hunter_config( Boost
    VERSION "1.64.0"
    CMAKE_ARGS
        BUILD_SHARED_LIBS:BOOL=${Boost_BUILD_SHARED_LIBS}
)
hunter_config(GTest
    VERSION ${HUNTER_GTest_VERSION}
    ${GTest_CMAKE_ARGS}
)
hunter_config( hdf5 
    VERSION ${HUNTER_hdf5_VERSION}
    CMAKE_ARGS
        BUILD_SHARED_LIBS:BOOL=ON
)

hunter_config( range-v3
    VERSION "0.5.0"
)
hunter_config(Nucleona
    VERSION ${HUNTER_Nucleona_VERSION}
    CMAKE_ARGS 
        BUILD_TESTS=OFF 
        ENABLE_HDF5=ON
)
hunter_config(ChipImgProc
    VERSION ${HUNTER_ChipImgProc_VERSION}
    CMAKE_ARGS 
        BUILD_TESTS=OFF 
)
hunter_config(CFU GIT_SUBMODULE "lib/CFU"
    CMAKE_ARGS
        BUILD_TESTS=OFF
)
hunter_config(Affy GIT_SUBMODULE "lib/Affy")

# OpenCV configs
hunter_config(
    Jpeg 
    VERSION ${HUNTER_Jpeg_VERSION}
    CMAKE_ARGS
        BUILD_SHARED_LIBS=ON
)
hunter_config(
    PNG
    VERSION ${HUNTER_PNG_VERSION}
    CMAKE_ARGS
        BUILD_SHARED_LIBS=ON
)
hunter_config(
    TIFF
    VERSION ${HUNTER_TIFF_VERSION}
    CMAKE_ARGS
        BUILD_SHARED_LIBS=ON
)
hunter_config(
    OpenCV
    VERSION "3.4.0-p0"
    CMAKE_ARGS 
        BUILD_SHARED_LIBS=ON
        ENABLE_PRECOMPILED_HEADERS=${OpenCV_ENABLE_PRECOMPILED_HEADERS}
)

hunter_config(
    CURL 
    VERSION ${HUNTER_CURL_VERSION}
    CMAKE_ARGS 
        CMAKE_USE_OPENSSL=${CURL_USE_OPENSSL}
        CMAKE_USE_WINSSL=${CURL_USE_WINSSL}
)
hunter_config(OpenBLAS
    VERSION ${OpenBLAS_VERSION}
)
