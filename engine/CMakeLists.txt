add_executable(rtype_engine src/main.cpp)
target_include_directories(rtype_engine PRIVATE include)
target_link_libraries(rtype_engine PRIVATE rtype_warnings)

if(RTYPE_BUILD_TESTS)
    add_test(NAME smoke_engine COMMAND rtype_engine)
endif()
