add_executable(r-type_server src/main.cpp)
target_include_directories(r-type_server PRIVATE include)
target_link_libraries(r-type_server PRIVATE rtype_warnings)

if(RTYPE_BUILD_TESTS)
    add_test(NAME smoke_server COMMAND r-type_server)
endif()
