/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CommandBuffer.cpp
*/

#include "Engine/Ecs/CommandBuffer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "Engine/Ecs/IndexedZipper.hpp"
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

TEST_CASE(
    "flush applies the commands in recording order and empties the buffer",
    "[ecs][command][flush]")
{
    Registry registry;
    std::vector<Entity> spawned;
    CommandBuffer buffer;

    registry.registerComponent<Position>();
    buffer.spawn([&](Entity entity) { spawned.push_back(entity); });
    buffer.spawn([&](Entity entity) { spawned.push_back(entity); });
    buffer.flush(registry);

    REQUIRE(buffer.empty());
    REQUIRE(registry.aliveCount() == 2);
    REQUIRE(spawned.size() == 2);
    REQUIRE(spawned[0] != spawned[1]);
}

TEST_CASE("the last recorded value of a component wins",
          "[ecs][command][flush]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<Position>();
    buffer.add<Position>(entity, Position{1, 1});
    buffer.emplace<Position>(entity, 2, 2);
    buffer.flush(registry);

    REQUIRE(registry.getComponents<Position>().get(entity)->x == 2);
}

TEST_CASE("a removal recorded after an addition removes the component",
          "[ecs][command][flush]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<Position>();
    buffer.add<Position>(entity, Position{1, 1});
    buffer.remove<Position>(entity);
    buffer.flush(registry);

    REQUIRE_FALSE(registry.getComponents<Position>().has(entity));
}

TEST_CASE("the recording order decides between a kill and an addition",
          "[ecs][command][flush]")
{
    Registry registry;
    const Entity addedFirst = registry.spawnEntity();
    const Entity killedFirst = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<Position>();
    buffer.add<Position>(addedFirst, Position{1, 1});
    buffer.kill(addedFirst);
    buffer.kill(killedFirst);
    buffer.add<Position>(killedFirst, Position{2, 2});
    buffer.flush(registry);

    REQUIRE_FALSE(registry.isAlive(addedFirst));
    REQUIRE_FALSE(registry.isAlive(killedFirst));
    REQUIRE_FALSE(registry.getComponents<Position>().has(addedFirst));
    REQUIRE_FALSE(registry.getComponents<Position>().has(killedFirst));
}

TEST_CASE("commands on a stale handle are ignored without error",
          "[ecs][command][flush]")
{
    Registry registry;
    const Entity stale = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<Position>();
    registry.killEntity(stale);
    buffer.kill(stale);
    buffer.kill(stale);
    buffer.add<Position>(stale, Position{1, 1});
    buffer.emplace<Position>(stale, 2, 2);
    buffer.remove<Position>(stale);

    REQUIRE_NOTHROW(buffer.flush(registry));
    REQUIRE(registry.aliveCount() == 0);
    REQUIRE(buffer.empty());
}

TEST_CASE("a stale handle does not hit the entity reusing its index",
          "[ecs][command][flush]")
{
    Registry registry;
    const Entity stale = registry.spawnEntity();
    CommandBuffer buffer;

    registry.killEntity(stale);
    const Entity reused = registry.spawnEntity();
    buffer.kill(stale);
    buffer.flush(registry);

    REQUIRE(reused.index() == stale.index());
    REQUIRE(registry.isAlive(reused));
}

TEST_CASE("the spawn callback receives a live handle", "[ecs][command][flush]")
{
    Registry registry;
    CommandBuffer buffer;
    bool alive = false;

    buffer.spawn([&](Entity entity) { alive = registry.isAlive(entity); });
    buffer.flush(registry);

    REQUIRE(alive);
}

TEST_CASE("a spawn without callback still creates the entity",
          "[ecs][command][flush]")
{
    Registry registry;
    CommandBuffer buffer;

    buffer.spawn();
    buffer.flush(registry);

    REQUIRE(registry.aliveCount() == 1);
}

TEST_CASE("flushing an empty buffer changes nothing", "[ecs][command][flush]")
{
    Registry registry;
    CommandBuffer buffer;

    buffer.flush(registry);

    REQUIRE(registry.aliveCount() == 0);
    REQUIRE(buffer.empty());
}

TEST_CASE("commands are never applied twice", "[ecs][command][flush]")
{
    Registry registry;
    CommandBuffer buffer;

    buffer.spawn();
    buffer.flush(registry);
    buffer.flush(registry);

    REQUIRE(registry.aliveCount() == 1);
}

TEST_CASE("killing entities while iterating is safe", "[ecs][command][flush]")
{
    Registry registry;
    std::vector<Entity> entities;
    CommandBuffer buffer;
    std::size_t visited = 0;

    registry.registerComponent<Position>();
    registry.registerComponent<Velocity>();
    for (int i = 0; i < 5; ++i) {
        entities.push_back(registry.spawnEntity());
        registry.addComponent(entities.back(), Position{i, 0});
        registry.addComponent(entities.back(), Velocity{1});
    }
    for (auto [index, position, velocity] :
         engine::ecs::makeIndexedZipper(registry.getComponents<Position>(),
                                        registry.getComponents<Velocity>())) {
        static_cast<void>(velocity);
        buffer.kill(entities[index]);
        buffer.spawn();
        position.y = 1;
        ++visited;
    }
    buffer.flush(registry);

    REQUIRE(visited == 5);
    REQUIRE(registry.aliveCount() == 5);
    REQUIRE_FALSE(registry.isAlive(entities[0]));
}

TEST_CASE("commands recorded during a flush wait for the next one",
          "[ecs][command][flush]")
{
    Registry registry;
    CommandBuffer buffer;
    Entity created = registry.spawnEntity();

    registry.killEntity(created);
    buffer.spawn([&](Entity entity) {
        created = entity;
        buffer.kill(entity);
    });
    buffer.flush(registry);
    REQUIRE(registry.isAlive(created));
    REQUIRE(buffer.size() == 1);
    buffer.flush(registry);

    REQUIRE_FALSE(registry.isAlive(created));
    REQUIRE(buffer.empty());
}

TEST_CASE("an unregistered component aborts the flush and empties the buffer",
          "[ecs][command][flush]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    buffer.add<Velocity>(entity, Velocity{1});
    buffer.kill(entity);

    REQUIRE_THROWS_AS(buffer.flush(registry), std::logic_error);
    REQUIRE(buffer.empty());
    REQUIRE(registry.isAlive(entity));
}

namespace {

class OwnedValue {
public:
    explicit OwnedValue(int value = 0)
        : _value(std::make_unique<int>(value))
    {
        ++alive;
    }

    OwnedValue(const OwnedValue &) = delete;
    OwnedValue &operator=(const OwnedValue &) = delete;

    OwnedValue(OwnedValue &&other) noexcept
        : _value(std::move(other._value))
    {
        ++alive;
    }

    OwnedValue &operator=(OwnedValue &&) noexcept = default;

    ~OwnedValue()
    {
        --alive;
    }

    [[nodiscard]] int value() const
    {
        return *_value;
    }

    static int alive;

private:
    std::unique_ptr<int> _value;
};

int OwnedValue::alive = 0;

static_assert(!std::is_copy_constructible_v<OwnedValue>);

} // namespace

TEST_CASE("a move-only component reaches the registry through add",
          "[ecs][command][owner]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<OwnedValue>();
    buffer.add<OwnedValue>(entity, OwnedValue(42));
    buffer.flush(registry);

    REQUIRE(registry.getComponents<OwnedValue>().get(entity)->value() == 42);
}

TEST_CASE("a move-only component reaches the registry through emplace",
          "[ecs][command][owner]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<OwnedValue>();
    buffer.emplace<OwnedValue>(entity, 7);
    buffer.flush(registry);

    REQUIRE(registry.getComponents<OwnedValue>().get(entity)->value() == 7);
}

TEST_CASE("the buffer owns a move-only component until it is applied",
          "[ecs][command][owner]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<OwnedValue>();
    buffer.add<OwnedValue>(entity, OwnedValue(1));
    REQUIRE(OwnedValue::alive == 1);
    buffer.flush(registry);

    REQUIRE(OwnedValue::alive == 1);
    registry.killEntity(entity);
    REQUIRE(OwnedValue::alive == 0);
}

TEST_CASE("clearing the buffer destroys a move-only component once",
          "[ecs][command][owner]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    buffer.add<OwnedValue>(entity, OwnedValue(1));
    buffer.emplace<OwnedValue>(entity, 2);
    REQUIRE(OwnedValue::alive == 2);
    buffer.clear();

    REQUIRE(OwnedValue::alive == 0);
}

TEST_CASE("a component applied to a stale handle is destroyed, not leaked",
          "[ecs][command][owner]")
{
    Registry registry;
    const Entity stale = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<OwnedValue>();
    registry.killEntity(stale);
    buffer.add<OwnedValue>(stale, OwnedValue(1));
    buffer.flush(registry);

    REQUIRE(OwnedValue::alive == 0);
}

TEST_CASE("components of an aborted flush are destroyed",
          "[ecs][command][owner]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    CommandBuffer buffer;

    registry.registerComponent<OwnedValue>();
    buffer.add<Velocity>(entity, Velocity{1});
    buffer.add<OwnedValue>(entity, OwnedValue(1));

    REQUIRE_THROWS_AS(buffer.flush(registry), std::logic_error);
    REQUIRE(OwnedValue::alive == 0);
}
