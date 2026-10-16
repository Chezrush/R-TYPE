add_executable(rtype_game src/main.cpp)
target_include_directories(rtype_game PRIVATE include)
target_link_libraries(rtype_game PRIVATE rtype_warnings)

if(RTYPE_BUILD_TESTS)
    add_test(NAME smoke_game COMMAND rtype_game)
endif()
