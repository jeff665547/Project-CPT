# message(STATUS "${__screw_rel_src_file} require CURL, Boost process 0.5, currently not compiled, skipped")
screw_extend_template()
target_link_libraries(${__screw_target} PUBLIC CURL::libcurl)