##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

add_library(rtype_engine STATIC
    src/Core/Version.cpp
    src/Ecs/Entity.cpp
    src/Ecs/EntityAllocator.cpp)
add_library(rtype::engine ALIAS rtype_engine)

target_include_directories(rtype_engine PUBLIC include)
target_compile_features(rtype_engine PUBLIC cxx_std_20)
target_link_libraries(rtype_engine PRIVATE rtype_warnings)

rtype_add_catch_test(rtype_engine_tests
    tests/Version.cpp
    tests/Ecs/Entity.cpp
    tests/Ecs/EntityAllocator.cpp
    tests/Ecs/IStorage.cpp
    tests/Ecs/SparseArray.cpp)
if(RTYPE_BUILD_TESTS)
    target_link_libraries(rtype_engine_tests PRIVATE rtype::engine)
endif()

if(RTYPE_BUILD_DEMOS)
    add_executable(rtype_engine_demo demo/Main.cpp)
    target_link_libraries(rtype_engine_demo PRIVATE rtype::engine rtype_warnings)
endif()
