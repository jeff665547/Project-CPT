screw_cmake_package_config_initial()
include(GNUInstallDirs)

write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)
install(TARGETS ${exe_targets} ${lib_targets} ${test_targets}
    EXPORT "${targets_export_name}"
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Library
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Library
)
install(
    DIRECTORY include
    DESTINATION . 
    COMPONENT Headers
)
if(BUILD_TESTS)
    install(
        DIRECTORY unit_test/data
        DESTINATION unit_test COMPONENT Tests
    )
endif()
install(
    FILES       "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install(
    EXPORT      "${targets_export_name}"
    NAMESPACE   "${namespace}"
    DESTINATION "${config_install_dir}"
)
if(INSTALL_DEPS)
    if(WIN32)
        configure_file(
            cmake/win/bundle.cmake.in 
            ${CMAKE_BINARY_DIR}/bundle.cmake @ONLY
        )
    elseif(UNIX)
        configure_file(
            cmake/lx/bundle.cmake.in
            ${CMAKE_BINARY_DIR}/bundle.cmake @ONLY
        )
    endif()
    install(
        SCRIPT ${CMAKE_BINARY_DIR}/bundle.cmake
        COMPONENT Dependencies
    )
endif()
screw_cmake_package_config_finish()
