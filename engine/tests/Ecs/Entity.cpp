/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Entity.cpp
*/

#include "Engine/Ecs/Entity.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <type_traits>

#include "Engine/Ecs/EntityAllocator.hpp"

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;

TEST_CASE("an entity exposes its index and generation", "[ecs][entity]")
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    const Entity second = allocator.create();

    REQUIRE(first.index() == 0);
    REQUIRE(first.generation() == 0);
    REQUIRE(second.index() == 1);
}

TEST_CASE("an entity converts to its index", "[ecs][entity]")
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    const Entity second = allocator.create();
    const std::size_t asIndex = second;

    REQUIRE(asIndex == 1);
    REQUIRE(static_cast<std::size_t>(first) == 0);
}

TEST_CASE("entities with another generation are different", "[ecs][entity]")
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    REQUIRE(allocator.destroy(first));
    const Entity reused = allocator.create();

    REQUIRE(first != reused);
    REQUIRE(static_cast<std::size_t>(first) ==
            static_cast<std::size_t>(reused));
}

TEST_CASE("entities are ordered by index then generation", "[ecs][entity]")
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    const Entity second = allocator.create();
    REQUIRE(allocator.destroy(second));
    const Entity reused = allocator.create();

    REQUIRE(first < second);
    REQUIRE(second < reused);
}

TEST_CASE("an entity cannot be built outside the allocator", "[ecs][entity]")
{
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<Entity, std::uint32_t, std::uint32_t>);
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<Entity>);
    STATIC_REQUIRE(std::is_trivially_copyable_v<Entity>);
}
