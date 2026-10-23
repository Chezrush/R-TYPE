/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CommandBuffer.cpp
*/

#include "Engine/Ecs/CommandBuffer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <utility>

#include "Engine/Ecs/Registry.hpp"

using engine::ecs::CommandBuffer;
using engine::ecs::Entity;
using engine::ecs::Registry;

namespace {

struct Position {
    int x;
    int y;
};

struct Velocity {
    int dx;
};

class Tracked {
public:
    Tracked()
    {
        ++alive;
    }

    Tracked(const Tracked &)
    {
        ++alive;
    }

    Tracked(Tracked &&) noexcept
    {
        ++alive;
    }

    Tracked &operator=(const Tracked &) = default;
    Tracked &operator=(Tracked &&) noexcept = default;

    ~Tracked()
    {
        --alive;
    }

    static int alive;
};

int Tracked::alive = 0;

} // namespace

TEST_CASE("a new command buffer is empty", "[ecs][command]")
{
    const CommandBuffer buffer;

    REQUIRE(buffer.empty());
    REQUIRE(buffer.size() == 0);
}

TEST_CASE("every operation is recorded as one command", "[ecs][command]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    buffer.spawn();
    REQUIRE(buffer.size() == 1);
    buffer.kill(entity);
    REQUIRE(buffer.size() == 2);
    buffer.add<Position>(entity, Position{1, 2});
    REQUIRE(buffer.size() == 3);
    buffer.emplace<Position>(entity, 3, 4);
    REQUIRE(buffer.size() == 4);
    buffer.remove<Velocity>(entity);
    REQUIRE(buffer.size() == 5);
    REQUIRE_FALSE(buffer.empty());
}

TEST_CASE("recording leaves the registry untouched", "[ecs][command]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<Position>();
    buffer.spawn();
    buffer.add<Position>(entity, Position{1, 2});
    buffer.kill(entity);

    REQUIRE(registry.isAlive(entity));
    REQUIRE(registry.aliveCount() == 1);
    REQUIRE_FALSE(registry.getComponents<Position>().has(entity));
}

TEST_CASE("clear drops every recorded command", "[ecs][command]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    buffer.spawn();
    buffer.kill(entity);
    buffer.clear();

    REQUIRE(buffer.empty());
    REQUIRE(buffer.size() == 0);
}

TEST_CASE("the buffer is reusable after a clear", "[ecs][command]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    buffer.kill(entity);
    buffer.clear();
    buffer.kill(entity);

    REQUIRE(buffer.size() == 1);
}

TEST_CASE("clear destroys the recorded components exactly once",
          "[ecs][command]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    buffer.add<Tracked>(entity, Tracked());
    buffer.emplace<Tracked>(entity);
    REQUIRE(Tracked::alive == 2);
    buffer.clear();

    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("destroying the buffer destroys the recorded components",
          "[ecs][command]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();

    {
        CommandBuffer buffer;
        buffer.add<Tracked>(entity, Tracked());
        REQUIRE(Tracked::alive == 1);
    }

    REQUIRE(Tracked::alive == 0);
}

TEST_CASE("a moved buffer keeps the recorded commands", "[ecs][command]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    buffer.kill(entity);
    buffer.kill(entity);
    const CommandBuffer moved = std::move(buffer);

    REQUIRE(moved.size() == 2);
}
