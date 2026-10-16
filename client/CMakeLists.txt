add_executable(r-type_client src/main.cpp)
target_include_directories(r-type_client PRIVATE include)
target_link_libraries(r-type_client PRIVATE rtype_warnings)

if(RTYPE_BUILD_TESTS)
    add_test(NAME smoke_client COMMAND r-type_client)
endif()
