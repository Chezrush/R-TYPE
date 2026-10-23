/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Registry.cpp
*/

#include "Engine/Ecs/Registry.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

using engine::ecs::Entity;
using engine::ecs::Registry;

namespace {

struct Position {
    int x;
    int y;
};

struct Name {
    std::string value;
};

using Owned = std::unique_ptr<int>;

} // namespace

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

TEST_CASE("a registered component type has a storage", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Registry &view = registry;

    REQUIRE(registry.getComponents<Position>().size() == 0);
    REQUIRE(view.getComponents<Position>().size() == 0);
}

TEST_CASE("an unregistered component type throws", "[ecs][registry]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();

    REQUIRE_THROWS_AS(registry.getComponents<Position>(), std::logic_error);
    REQUIRE_THROWS_AS(registry.addComponent(entity, Position{1, 2}),
                      std::logic_error);
    REQUIRE_THROWS_AS(registry.removeComponent<Position>(entity),
                      std::logic_error);
}

TEST_CASE("registering a type twice keeps its components", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Entity entity = registry.spawnEntity();
    registry.addComponent(entity, Position{1, 2});
    registry.registerComponent<Position>();

    REQUIRE(registry.getComponents<Position>().has(entity));
}

TEST_CASE("components are added, read and removed", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Entity entity = registry.spawnEntity();

    Position *added = registry.addComponent(entity, Position{3, 4});
    REQUIRE(added != nullptr);
    REQUIRE(registry.getComponents<Position>().get(entity)->x == 3);
    registry.removeComponent<Position>(entity);
    REQUIRE_FALSE(registry.getComponents<Position>().has(entity));
}

TEST_CASE("a component is built in place", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Name>();
    const Entity entity = registry.spawnEntity();

    Name *name = registry.emplaceComponent<Name>(entity, "player");
    REQUIRE(name->value == "player");
    REQUIRE(registry.getComponents<Name>().get(entity) == name);
}

TEST_CASE("a move-only component is supported", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Owned>();
    const Entity entity = registry.spawnEntity();

    registry.addComponent(entity, std::make_unique<int>(7));
    registry.emplaceComponent<Owned>(entity, std::make_unique<int>(8));
    REQUIRE(**registry.getComponents<Owned>().get(entity) == 8);
}

TEST_CASE("different component types coexist", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    registry.registerComponent<Name>();
    const Entity entity = registry.spawnEntity();

    registry.addComponent(entity, Position{1, 1});
    registry.removeComponent<Name>(entity);
    REQUIRE(registry.getComponents<Position>().has(entity));
    REQUIRE_FALSE(registry.getComponents<Name>().has(entity));
}

TEST_CASE("killing an entity removes all of its components", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    registry.registerComponent<Name>();
    const Entity entity = registry.spawnEntity();
    const Entity other = registry.spawnEntity();
    registry.addComponent(entity, Position{1, 1});
    registry.emplaceComponent<Name>(entity, "gone");
    registry.addComponent(other, Position{2, 2});

    REQUIRE(registry.killEntity(entity));
    REQUIRE_FALSE(registry.getComponents<Position>().has(entity));
    REQUIRE_FALSE(registry.getComponents<Name>().has(entity));
    REQUIRE(registry.getComponents<Position>().has(other));
}

TEST_CASE("a reused index starts without components", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Entity old = registry.spawnEntity();
    registry.addComponent(old, Position{1, 1});
    REQUIRE(registry.killEntity(old));
    const Entity reused = registry.spawnEntity();

    REQUIRE(reused.index() == old.index());
    REQUIRE_FALSE(registry.getComponents<Position>().has(reused));
}

TEST_CASE("a stale handle gets no component", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Entity old = registry.spawnEntity();
    REQUIRE(registry.killEntity(old));
    const Entity reused = registry.spawnEntity();

    REQUIRE(registry.addComponent(old, Position{1, 1}) == nullptr);
    REQUIRE(registry.emplaceComponent<Position>(old, 1, 1) == nullptr);
    REQUIRE_FALSE(registry.getComponents<Position>().has(reused));
}

TEST_CASE("an unregistered type throws before the handle is checked",
          "[ecs][registry]")
{
    Registry registry;
    const Registry &view = registry;
    const Entity stale = registry.spawnEntity();
    REQUIRE(registry.killEntity(stale));

    REQUIRE_THROWS_AS(registry.addComponent(stale, Position{1, 1}),
                      std::logic_error);
    REQUIRE_THROWS_AS(registry.emplaceComponent<Position>(stale, 1, 1),
                      std::logic_error);
    REQUIRE_THROWS_AS(registry.removeComponent<Position>(stale),
                      std::logic_error);
    REQUIRE_THROWS_AS(view.getComponents<Position>(), std::logic_error);
}

TEST_CASE("removing through a stale handle spares the new owner",
          "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Entity old = registry.spawnEntity();
    REQUIRE(registry.killEntity(old));
    const Entity reused = registry.spawnEntity();
    registry.addComponent(reused, Position{5, 5});

    registry.removeComponent<Position>(old);
    REQUIRE(registry.getComponents<Position>().has(reused));
}

TEST_CASE("killing a stale handle spares the new owner", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Entity old = registry.spawnEntity();
    REQUIRE(registry.killEntity(old));
    const Entity reused = registry.spawnEntity();
    registry.addComponent(reused, Position{5, 5});

    REQUIRE_FALSE(registry.killEntity(old));
    REQUIRE(registry.isAlive(reused));
    REQUIRE(registry.getComponents<Position>().has(reused));
}

TEST_CASE("a stale handle never throws", "[ecs][registry]")
{
    Registry registry;
    registry.registerComponent<Position>();
    const Entity old = registry.spawnEntity();
    REQUIRE(registry.killEntity(old));

    REQUIRE_NOTHROW(registry.addComponent(old, Position{1, 1}));
    REQUIRE_NOTHROW(registry.emplaceComponent<Position>(old, 1, 1));
    REQUIRE_NOTHROW(registry.removeComponent<Position>(old));
    REQUIRE_NOTHROW(registry.killEntity(old));
    REQUIRE_NOTHROW(registry.isAlive(old));
}

TEST_CASE("entityAt gives back the handle of a spawned entity",
          "[ecs][registry]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();

    REQUIRE(registry.entityAt(entity.index()) == entity);
    REQUIRE_THROWS_AS(registry.entityAt(5), std::out_of_range);
}
