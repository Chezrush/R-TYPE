##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

add_library(rtype_engine STATIC
    src/Core/EventBus.cpp
    src/Core/FixedTimestep.cpp
    src/Core/GameLoop.cpp
    src/Core/Vec2.cpp
    src/Core/SceneManager.cpp
    src/Core/Version.cpp
    src/Ecs/Access.cpp
    src/Ecs/CommandBuffer.cpp
    src/Ecs/Entity.cpp
    src/Ecs/EntityAllocator.cpp
    src/Ecs/Phase.cpp
    src/Ecs/Scheduler.cpp
    src/Ecs/Registry.cpp
    src/Ecs/Components/UITransform.cpp
    src/Ecs/Components/UIComponents.cpp
    src/Ecs/Systems/UIInteractionSystem.cpp
    src/Ecs/Systems/UIWidgetInput.cpp
    src/Ecs/Systems/UIWidgetLogic.cpp
    src/Ecs/Systems/UILayoutSystem.cpp)
add_library(rtype::engine ALIAS rtype_engine)

target_include_directories(rtype_engine PUBLIC include)
target_compile_features(rtype_engine PUBLIC cxx_std_20)
target_link_libraries(rtype_engine PRIVATE rtype_warnings)

rtype_add_catch_test(rtype_engine_tests
    tests/Core/SceneManager.cpp
    tests/Version.cpp
    tests/Core/EventBus.cpp
    tests/Core/FixedTimestep.cpp
    tests/Core/GameLoop.cpp
    tests/Core/Vec2.cpp
    tests/Ecs/Access.cpp
    tests/Ecs/CommandBuffer.cpp
    tests/Ecs/Entity.cpp
    tests/Ecs/EntityAllocator.cpp
    tests/Ecs/IndexedZipper.cpp
    tests/Ecs/IStorage.cpp
    tests/Ecs/Phase.cpp
    tests/Ecs/Scheduler.cpp
    tests/Ecs/Registry.cpp
    tests/Ecs/SparseArray.cpp
    tests/Ecs/Zipper.cpp
    tests/Ecs/ZipperConcepts.cpp
    tests/Ecs/Systems/UIInteractionSystem.cpp
    tests/Ecs/Systems/UIWidgetLogic.cpp
    tests/Ecs/Systems/UIWidgets.cpp
    tests/Ecs/Systems/UILayoutSystem.cpp
    tests/Resources/ResourceCache.cpp)
if(RTYPE_BUILD_TESTS)
    target_link_libraries(rtype_engine_tests PRIVATE rtype::engine)
endif()

# Rendering needs SFML, so it lives in its own library: the server only links
# rtype::engine and never pulls SFML in.
if(RTYPE_BUILD_CLIENT)
    find_package(SFML 3 COMPONENTS Graphics CONFIG REQUIRED)

    add_library(rtype_engine_render STATIC
        src/Render/UIRenderSystem.cpp
        src/Resources/ResourceManager.cpp)
    add_library(rtype::engine_render ALIAS rtype_engine_render)
    target_link_libraries(rtype_engine_render
        PUBLIC rtype::engine SFML::Graphics
        PRIVATE rtype_warnings)

    rtype_add_catch_test(rtype_engine_render_tests
        tests/Render/UIRenderSystem.cpp
        tests/Resources/ResourceManager.cpp)
    if(RTYPE_BUILD_TESTS)
        target_link_libraries(rtype_engine_render_tests
            PRIVATE rtype::engine_render)
        target_compile_definitions(rtype_engine_render_tests
            PRIVATE RTYPE_TEST_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")
    endif()
endif()

if(RTYPE_BUILD_DEMOS)
    add_executable(rtype_engine_demo demo/Main.cpp)
    target_link_libraries(rtype_engine_demo PRIVATE rtype::engine rtype_warnings)
endif()

if(RTYPE_BUILD_BENCHMARKS)
    add_executable(rtype_engine_bench
        bench/Cases.cpp
        bench/Main.cpp
        bench/Series.cpp)
    target_link_libraries(rtype_engine_bench PRIVATE rtype::engine rtype_warnings)
endif()
