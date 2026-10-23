/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EntityAllocator.cpp
*/

#include "Engine/Ecs/EntityAllocator.hpp"

#include <catch2/catch_test_macros.hpp>
#include <set>
#include <stdexcept>

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;

TEST_CASE("a created entity is alive", "[ecs][allocator]")
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();

    REQUIRE(allocator.isAlive(entity));
    REQUIRE(allocator.aliveCount() == 1);
}

TEST_CASE("live entities have distinct handles", "[ecs][allocator]")
{
    EntityAllocator allocator;
    std::set<Entity> handles;

    for (int i = 0; i < 100; ++i)
        handles.insert(allocator.create());
    REQUIRE(handles.size() == 100);
    REQUIRE(allocator.aliveCount() == 100);
}

TEST_CASE("a destroyed entity is not alive", "[ecs][allocator]")
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();

    REQUIRE(allocator.destroy(entity));
    REQUIRE_FALSE(allocator.isAlive(entity));
    REQUIRE(allocator.aliveCount() == 0);
}

TEST_CASE("destroying a stale handle changes nothing", "[ecs][allocator]")
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();
    const Entity other = allocator.create();

    REQUIRE(allocator.destroy(entity));
    REQUIRE_FALSE(allocator.destroy(entity));
    REQUIRE(allocator.isAlive(other));
    REQUIRE(allocator.aliveCount() == 1);
}

TEST_CASE("a stale handle stays dead after the index is reused",
          "[ecs][allocator]")
{
    EntityAllocator allocator;
    const Entity old = allocator.create();
    REQUIRE(allocator.destroy(old));
    const Entity reused = allocator.create();

    REQUIRE(reused.index() == old.index());
    REQUIRE(reused.generation() == old.generation() + 1);
    REQUIRE(allocator.isAlive(reused));
    REQUIRE_FALSE(allocator.isAlive(old));
    REQUIRE_FALSE(allocator.destroy(old));
    REQUIRE(allocator.isAlive(reused));
}

TEST_CASE("freed indices are recycled in LIFO order", "[ecs][allocator]")
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    const Entity second = allocator.create();
    const Entity third = allocator.create();

    REQUIRE(allocator.destroy(first));
    REQUIRE(allocator.destroy(third));
    REQUIRE(allocator.destroy(second));
    REQUIRE(allocator.create().index() == second.index());
    REQUIRE(allocator.create().index() == third.index());
    REQUIRE(allocator.create().index() == first.index());
}

TEST_CASE("a handle that was never allocated is not alive", "[ecs][allocator]")
{
    EntityAllocator allocator;
    EntityAllocator other;
    static_cast<void>(other.create());
    const Entity unknown = other.create();

    REQUIRE_FALSE(allocator.isAlive(unknown));
    REQUIRE_FALSE(allocator.destroy(unknown));
    REQUIRE(allocator.aliveCount() == 0);
}

TEST_CASE("handleAt returns the handle using an index", "[ecs][allocator]")
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    const Entity second = allocator.create();

    REQUIRE(allocator.handleAt(0) == first);
    REQUIRE(allocator.handleAt(1) == second);
}

TEST_CASE("handleAt follows the generation of a reused index",
          "[ecs][allocator]")
{
    EntityAllocator allocator;
    const Entity old = allocator.create();

    allocator.destroy(old);
    const Entity reused = allocator.create();

    REQUIRE(allocator.handleAt(old.index()) == reused);
    REQUIRE(allocator.handleAt(old.index()) != old);
}

TEST_CASE("handleAt rejects an index never allocated", "[ecs][allocator]")
{
    EntityAllocator allocator;

    static_cast<void>(allocator.create());

    REQUIRE_THROWS_AS(allocator.handleAt(1), std::out_of_range);
}
