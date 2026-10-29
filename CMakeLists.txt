##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

cmake_minimum_required(VERSION 3.21)

project(r-type
    VERSION 0.1.0
    DESCRIPTION "A networked R-Type clone built on a custom game engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(RTYPE_BUILD_SERVER "Build r-type_server" ON)
option(RTYPE_BUILD_CLIENT "Build r-type_client (needs SFML)" ON)
option(RTYPE_BUILD_TESTS "Build the unit tests" ON)
option(RTYPE_BUILD_DEMOS "Build the engine demo programs" ON)
option(RTYPE_BUILD_BENCHMARKS "Build the engine benchmarks (not run by ctest)" OFF)

find_package(Threads REQUIRED)

# Warnings are shared by every target through this interface library.
add_library(rtype_warnings INTERFACE)
if(MSVC)
    target_compile_options(rtype_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(rtype_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

if(RTYPE_BUILD_TESTS)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
    include(Catch)
    add_library(rtype_test_support INTERFACE)
    target_include_directories(rtype_test_support INTERFACE ${CMAKE_SOURCE_DIR}/tests/include)
endif()

function(rtype_add_test name)
    if(RTYPE_BUILD_TESTS)
        add_executable(${name} ${ARGN})
        target_link_libraries(${name} PRIVATE rtype_test_support rtype_warnings)
        add_test(NAME ${name} COMMAND ${name})
    endif()
endfunction()

# Declares a Catch2 test executable; every TEST_CASE becomes its own CTest test.
function(rtype_add_catch_test name)
    if(RTYPE_BUILD_TESTS)
        add_executable(${name} ${ARGN})
        target_link_libraries(${name} PRIVATE Catch2::Catch2WithMain rtype_warnings)
        catch_discover_tests(${name})
    endif()
endfunction()

# Layering (a module may only depend on the ones above it):
#   engine   -> generic, knows nothing about R-Type
#   protocol -> wire format shared by client and server
#   game     -> R-Type components, systems and rules (engine + protocol)
#   server / client -> executables
add_subdirectory(engine)
add_subdirectory(protocol)
add_subdirectory(game)

if(RTYPE_BUILD_SERVER)
    add_subdirectory(server)
endif()
if(RTYPE_BUILD_CLIENT)
    add_subdirectory(client)
endif()
