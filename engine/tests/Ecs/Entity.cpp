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

namespace engine::ecs {

class EntityAllocator {
public:
    static Entity make(std::uint32_t index, std::uint32_t generation)
    {
        return Entity(index, generation);
    }
};

} // namespace engine::ecs

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;

TEST_CASE("an entity exposes its index and generation", "[ecs][entity]")
{
    const Entity entity = EntityAllocator::make(7, 3);

    REQUIRE(entity.index() == 7);
    REQUIRE(entity.generation() == 3);
}

TEST_CASE("an entity converts to its index", "[ecs][entity]")
{
    const Entity entity = EntityAllocator::make(7, 3);
    const std::size_t asIndex = entity;

    REQUIRE(asIndex == 7);
}

TEST_CASE("entities with another generation are different", "[ecs][entity]")
{
    const Entity first = EntityAllocator::make(7, 0);
    const Entity reused = EntityAllocator::make(7, 1);

    REQUIRE(first != reused);
    REQUIRE(first == EntityAllocator::make(7, 0));
    REQUIRE(static_cast<std::size_t>(first) ==
            static_cast<std::size_t>(reused));
}

TEST_CASE("entities are ordered by index then generation", "[ecs][entity]")
{
    REQUIRE(EntityAllocator::make(1, 5) < EntityAllocator::make(2, 0));
    REQUIRE(EntityAllocator::make(2, 0) < EntityAllocator::make(2, 1));
}

TEST_CASE("an entity cannot be built outside the allocator", "[ecs][entity]")
{
    STATIC_REQUIRE_FALSE(
        std::is_constructible_v<Entity, std::uint32_t, std::uint32_t>);
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<Entity>);
    STATIC_REQUIRE(std::is_trivially_copyable_v<Entity>);
}
