##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

# The R-Type world. It knows nothing about the network: no sockets, packets or
# client ids. The server links it and drives it one fixed tick at a time.
add_library(rtype_game STATIC
    src/Bounds.cpp
    src/Collision.cpp
    src/Components.cpp
    src/Entities.cpp
    src/NetIdAllocator.cpp
    src/Phases.cpp
    src/Players.cpp
    src/Random.cpp
    src/ShipMovement.cpp
    src/Simulation.cpp
    src/Waves.cpp
    src/Systems/BoundsSystem.cpp
    src/Systems/CollisionSystem.cpp
    src/Systems/DamageSystem.cpp
    src/Systems/EnemyAISystem.cpp
    src/Systems/GameStateSystem.cpp
    src/Systems/InputSystem.cpp
    src/Systems/MovementSystem.cpp
    src/Systems/RespawnSystem.cpp
    src/Systems/SpawnSystem.cpp)
add_library(rtype::game ALIAS rtype_game)

target_include_directories(rtype_game PUBLIC include)
target_compile_features(rtype_game PUBLIC cxx_std_20)
target_link_libraries(rtype_game
    PUBLIC rtype::engine
    PRIVATE rtype_warnings)

rtype_add_catch_test(rtype_game_tests
    tests/Bounds.cpp
    tests/Collision.cpp
    tests/Combat.cpp
    tests/Components.cpp
    tests/Enemies.cpp
    tests/Lives.cpp
    tests/NetIdAllocator.cpp
    tests/OffScreen.cpp
    tests/Phases.cpp
    tests/Players.cpp
    tests/Random.cpp
    tests/ShipMovement.cpp
    tests/Simulation.cpp
    tests/Waves.cpp)
if(RTYPE_BUILD_TESTS)
    target_link_libraries(rtype_game_tests PRIVATE rtype::game)
endif()
