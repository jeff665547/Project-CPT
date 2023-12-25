screw_extend_template()
target_link_libraries(
    ${__screw_target} PRIVATE
    CPT-engine-data_pool-component_object_manager-cpsym_tab
    mlpack::mlpack
)
target_compile_definitions(${__screw_target} PUBLIC NO_VTK NEW_DATA_POOL)
if(MINGW)
    target_compile_options(${__screw_target} PUBLIC -Wa,-mbig-obj)
endif()
