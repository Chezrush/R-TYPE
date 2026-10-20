##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

add_executable(r-type_server 
src/main.cpp
src/Server.cpp
)
target_include_directories(r-type_server PRIVATE include)
target_link_libraries(r-type_server PRIVATE rtype_warnings)
target_compile_features(r-type_server PUBLIC cxx_std_20)

if(RTYPE_BUILD_TESTS)
    add_test(NAME smoke_server COMMAND r-type_server)
endif()
