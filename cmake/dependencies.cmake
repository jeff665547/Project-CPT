# nucleona 
hunter_add_package(Nucleona)
find_package(Nucleona CONFIG REQUIRED)

# chip image process 
hunter_add_package(ChipImgProc)
find_package(ChipImgProc CONFIG REQUIRED)

# range v3
hunter_add_package(range-v3)
find_package(range-v3 CONFIG REQUIRED)
if(MSVC)
    target_compile_options(range-v3 INTERFACE /permissive-)
endif()

# boost 
hunter_add_package(Boost COMPONENTS 
    thread 
    system 
    filesystem
    # more boost module goes here
)
find_package(Boost CONFIG COMPONENTS 
    thread 
    system 
    filesystem 
    # more boost module goes here
    REQUIRED
)

hunter_add_package(BoostProcess)
find_package(BoostProcess CONFIG COMPONENTS)

# gtest
if(BUILD_TESTS)
    hunter_add_package(GTest)
    find_package(GTest CONFIG REQUIRED)
endif()

# CFU
hunter_add_package(CFU)
find_package(CFU CONFIG REQUIRED)

# mlpack
hunter_add_package(mlpack)
find_package(mlpack CONFIG REQUIRED)

# Affy
hunter_add_package(Affy)
find_package(Affy CONFIG REQUIRED)

# libsimdpp
hunter_add_package(libsimdpp)
find_package(libsimdpp CONFIG REQUIRED)

# spdlog
hunter_add_package(spdlog)
find_package(spdlog)

# OpenCV
screw_get_bits(BITS)
if(WIN32)
    list(APPEND BUNDLE_RT_DIRS ${OpenCV_DIR}/x${BITS}/${OpenCV_RUNTIME}/bin)
    list(APPEND BUNDLE_RT_DIRS ${OpenCV_DIR}/x${BITS}/${OpenCV_RUNTIME}/lib)
else()
endif()

# CURL
hunter_add_package(CURL)
find_package(CURL CONFIG REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)


include(${SCREW_DIR}/hunter_root.cmake)
