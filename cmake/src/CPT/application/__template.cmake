screw_path_to_target_name(__screw_rel_src_file __screw_target)
screw_add_executable(${__screw_target} ${__screw_src_file})
target_include_directories(${__screw_target} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(${__screw_target} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/CCD/include>
    $<INSTALL_INTERFACE:lib/CCD/include>
)
target_include_directories(${__screw_target} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/lib/slam_quant_seq/include>
    $<INSTALL_INTERFACE:lib/slam_quant_seq/include>
)
target_link_libraries(${__screw_target} PUBLIC
    Boost::system
    Boost::filesystem 
    Nucleona::Nucleona
    Boost::program_options
    Threads::Threads
    CPT-logger
)
screw_add_launch_task(${__screw_target})

screw_show_var(__screw_target)