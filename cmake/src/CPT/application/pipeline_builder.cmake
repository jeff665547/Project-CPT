# message(STATUS "${__screw_rel_src_file} require VTK, currently not compiled, skipped")
screw_extend_template()
target_compile_definitions(${__screw_target} PUBLIC NO_VTK NEW_DATA_POOL)
if(MINGW)
    target_compile_options(${__screw_target} PUBLIC -Wa,-mbig-obj)
endif()
target_link_libraries(${__screw_target} PRIVATE
    CPT-engine-data_pool-component_object_manager-cpsym_tab
    CPT-utility
    mlpack::mlpack
)
# target_sources(${__screw_target} PRIVATE
#     src/CPT/engine/data_pool/component_object_manager/cpsym_tab.cpp 
# )
screw_add_launch_task(${__screw_target})
