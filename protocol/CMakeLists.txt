##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

add_executable(rtype_protocol src/main.cpp)
target_include_directories(rtype_protocol PRIVATE include)
target_link_libraries(rtype_protocol PRIVATE rtype_warnings)

if(RTYPE_BUILD_TESTS)
    add_test(NAME smoke_protocol COMMAND rtype_protocol)
endif()
