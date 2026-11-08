##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

# Client code without SFML (HUD, menu scenes), shared by the executable
# and the tests.
add_library(rtype_client_core STATIC
    src/Console/ConsoleModel.cpp
    src/Console/ConsoleService.cpp
    src/Hud/BeamGauge.cpp
    src/Hud/HudController.cpp
    src/Hud/HudView.cpp
    src/Hud/StatsOverlay.cpp
    src/Hud/VisualCueIndicator.cpp
    src/Scenes/ConnectForm.cpp
    src/Lobby/ChatModel.cpp
    src/Lobby/ChatService.cpp
    src/Lobby/LobbyText.cpp
    src/Net/ClientSession.cpp
    src/Scenes/ChatOverlay.cpp
    src/Scenes/ConnectScene.cpp
    src/Scenes/ConsoleOverlay.cpp
    src/Scenes/ControlsScene.cpp
    src/Scenes/GameOverOverlay.cpp
    src/Scenes/GameOverlays.cpp
    src/Scenes/HowToPlayScene.cpp
    src/Scenes/InGameMenuOverlay.cpp
    src/Scenes/LobbyScene.cpp
    src/Scenes/MainMenuScene.cpp
    src/Scenes/MenuContext.cpp
    src/Scenes/RespawnOverlay.cpp
    src/Scenes/RoomBrowserScene.cpp
    src/Scenes/SettingsScene.cpp
    src/Scenes/Starfield.cpp
    src/Settings/ClientSettings.cpp
    src/Settings/KeyBindings.cpp
    src/Settings/Palette.cpp
    src/Settings/SettingsStore.cpp
    src/Stats/StatsModel.cpp
    src/World/WorldState.cpp)
target_include_directories(rtype_client_core PUBLIC include)
find_package(asio CONFIG REQUIRED)
target_link_libraries(rtype_client_core
    PUBLIC rtype::engine rtype_protocol rtype::game asio::asio Threads::Threads
    PRIVATE rtype_warnings)

# Everything that knows the window library: drawing, keyboard and sound.
find_package(SFML 3 COMPONENTS Graphics Audio CONFIG REQUIRED)
add_library(rtype_client_render STATIC src/Render/GameWindow.cpp)
target_include_directories(rtype_client_render PUBLIC include)
target_link_libraries(rtype_client_render
    PUBLIC rtype_client_core SFML::Graphics SFML::Audio
    PRIVATE rtype_warnings)

add_executable(r-type_client src/main.cpp)
target_include_directories(r-type_client PRIVATE include)
target_link_libraries(r-type_client PRIVATE rtype_client_render rtype_warnings)

rtype_add_catch_test(rtype_client_tests
    tests/Console/ConsoleModel.cpp
    tests/Hud/BeamGauge.cpp
    tests/Hud/HudController.cpp
    tests/Hud/HudView.cpp
    tests/Hud/StatsOverlay.cpp
    tests/Hud/VisualCueIndicator.cpp
    tests/Lobby/ChatModel.cpp
    tests/Lobby/LobbyText.cpp
    tests/Scenes/ConnectForm.cpp
    tests/Scenes/ConnectScene.cpp
    tests/Scenes/ConsoleOverlay.cpp
    tests/Scenes/ControlsScene.cpp
    tests/Scenes/GameOverlays.cpp
    tests/Scenes/LobbyScenes.cpp
    tests/Scenes/MainMenuScene.cpp
    tests/Scenes/SceneRouter.cpp
    tests/Scenes/SettingsScene.cpp
    tests/Scenes/UIScaleFit.cpp
    tests/Settings/ClientSettings.cpp
    tests/Settings/KeyBindings.cpp
    tests/Settings/Palette.cpp
    tests/Settings/SettingsStore.cpp
    tests/Stats/StatsModel.cpp
    tests/World/WorldState.cpp)
if(RTYPE_BUILD_TESTS)
    target_link_libraries(rtype_client_tests PRIVATE rtype_client_core)
    target_include_directories(rtype_client_tests PRIVATE tests/Scenes
        tests/Settings)
    add_test(NAME smoke_client COMMAND r-type_client --help)
endif()

# Talks to a real server on the loopback.
if(RTYPE_BUILD_TESTS AND RTYPE_BUILD_SERVER)
    rtype_add_catch_test(rtype_client_net_tests tests/Net/ClientSession.cpp)
    target_link_libraries(rtype_client_net_tests
        PRIVATE rtype_client_core rtype_server_core)
endif()
