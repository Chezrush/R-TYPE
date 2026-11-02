##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

# Client code without SFML (HUD, menu scenes), shared by the executable
# and the tests.
add_library(rtype_client_core STATIC
    src/Hud/BeamGauge.cpp
    src/Hud/HudController.cpp
    src/Hud/HudView.cpp
    src/Scenes/ConnectForm.cpp
    src/Scenes/ConnectScene.cpp
    src/Scenes/GameOverOverlay.cpp
    src/Scenes/GameOverlays.cpp
    src/Scenes/InGameMenuOverlay.cpp
    src/Scenes/MainMenuScene.cpp
    src/Scenes/MenuContext.cpp
    src/Scenes/RespawnOverlay.cpp
    src/Scenes/Starfield.cpp)
target_include_directories(rtype_client_core PUBLIC include)
target_link_libraries(rtype_client_core
    PUBLIC rtype::engine
    PRIVATE rtype_warnings)

add_executable(r-type_client src/main.cpp)
target_include_directories(r-type_client PRIVATE include)
target_link_libraries(r-type_client PRIVATE rtype_warnings)

rtype_add_catch_test(rtype_client_tests
    tests/Hud/BeamGauge.cpp
    tests/Hud/HudController.cpp
    tests/Hud/HudView.cpp
    tests/Scenes/ConnectForm.cpp
    tests/Scenes/ConnectScene.cpp
    tests/Scenes/GameOverlays.cpp
    tests/Scenes/MainMenuScene.cpp
    tests/Scenes/SceneRouter.cpp)
if(RTYPE_BUILD_TESTS)
    target_link_libraries(rtype_client_tests PRIVATE rtype_client_core)
    target_include_directories(rtype_client_tests PRIVATE tests/Scenes)
    add_test(NAME smoke_client COMMAND r-type_client)
endif()
