/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Registry.cpp
*/

#include "Engine/Ecs/Registry.hpp"

#include <catch2/catch_test_macros.hpp>
#include <type_traits>

using engine::ecs::Entity;
using engine::ecs::Registry;

TEST_CASE("a spawned entity is alive", "[ecs][registry]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();

    REQUIRE(registry.isAlive(entity));
    REQUIRE(registry.aliveCount() == 1);
}

TEST_CASE("a killed entity is not alive", "[ecs][registry]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();

    REQUIRE(registry.killEntity(entity));
    REQUIRE_FALSE(registry.isAlive(entity));
    REQUIRE(registry.aliveCount() == 0);
}

TEST_CASE("killing a stale handle is reported and changes nothing",
          "[ecs][registry]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    const Entity other = registry.spawnEntity();

    REQUIRE(registry.killEntity(entity));
    REQUIRE_FALSE(registry.killEntity(entity));
    REQUIRE(registry.isAlive(other));
    REQUIRE(registry.aliveCount() == 1);
}

TEST_CASE("a reused index does not revive the old handle", "[ecs][registry]")
{
    Registry registry;
    const Entity old = registry.spawnEntity();
    REQUIRE(registry.killEntity(old));
    const Entity reused = registry.spawnEntity();

    REQUIRE(reused.index() == old.index());
    REQUIRE(registry.isAlive(reused));
    REQUIRE_FALSE(registry.isAlive(old));
    REQUIRE_FALSE(registry.killEntity(old));
}

TEST_CASE("the registry cannot be copied", "[ecs][registry]")
{
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<Registry>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<Registry>);
    STATIC_REQUIRE(std::is_move_constructible_v<Registry>);
}
