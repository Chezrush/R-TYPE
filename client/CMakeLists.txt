##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

# Client code without SFML (HUD state and widgets), shared by the executable
# and the tests.
add_library(rtype_client_core STATIC
    src/Hud/BeamGauge.cpp
    src/Hud/HudController.cpp
    src/Hud/HudView.cpp)
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
    tests/Hud/HudView.cpp)
if(RTYPE_BUILD_TESTS)
    target_link_libraries(rtype_client_tests PRIVATE rtype_client_core)
    add_test(NAME smoke_client COMMAND r-type_client)
endif()
