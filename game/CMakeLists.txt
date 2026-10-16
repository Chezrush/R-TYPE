##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

add_executable(rtype_game src/main.cpp)
target_include_directories(rtype_game PRIVATE include)
target_link_libraries(rtype_game PRIVATE rtype_warnings)

if(RTYPE_BUILD_TESTS)
    add_test(NAME smoke_game COMMAND rtype_game)
endif()
