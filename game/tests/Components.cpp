/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Components.cpp
*/

#include "Rtype/Components/Components.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "Rtype/GameRules.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;
using rtype::game::EnemyAI;
using rtype::game::GameRules;
using rtype::game::Health;
using rtype::game::Hitbox;
using rtype::game::Kind;
using rtype::game::Lives;
using rtype::game::NetId;
using rtype::game::NO_PLAYER;
using rtype::game::PlayerControl;
using rtype::game::Position;
using rtype::game::registerComponents;
using rtype::game::Team;
using rtype::game::TeamSide;
using rtype::game::Velocity;

using AllComponents = std::tuple<Position, Velocity, Hitbox, NetId, Kind,
                                 PlayerControl, Health, Lives, Team, EnemyAI>;

TEMPLATE_LIST_TEST_CASE("a component is a plain value type",
                        "[game][components]", AllComponents)
{
    STATIC_REQUIRE(std::is_aggregate_v<TestType>);
    STATIC_REQUIRE(std::is_trivially_copyable_v<TestType>);
    STATIC_REQUIRE(std::is_default_constructible_v<TestType>);
    REQUIRE(TestType{} == TestType{});
}

TEMPLATE_LIST_TEST_CASE("a component can be registered and attached",
                        "[game][components]", AllComponents)
{
    Registry registry;
    registry.registerComponent<TestType>();
    const Entity entity = registry.spawnEntity();

    REQUIRE(registry.addComponent(entity, TestType{}) != nullptr);
    REQUIRE(registry.getComponents<TestType>().has(entity));
    REQUIRE(*registry.getComponents<TestType>().get(entity) == TestType{});
}

TEST_CASE("registerComponents registers every component", "[game][components]")
{
    Registry registry;
    registerComponents(registry);

    REQUIRE(registry.getComponents<Position>().size() == 0);
    REQUIRE(registry.getComponents<Velocity>().size() == 0);
    REQUIRE(registry.getComponents<Hitbox>().size() == 0);
    REQUIRE(registry.getComponents<NetId>().size() == 0);
    REQUIRE(registry.getComponents<Kind>().size() == 0);
    REQUIRE(registry.getComponents<PlayerControl>().size() == 0);
    REQUIRE(registry.getComponents<Health>().size() == 0);
    REQUIRE(registry.getComponents<Lives>().size() == 0);
    REQUIRE(registry.getComponents<Team>().size() == 0);
    REQUIRE(registry.getComponents<EnemyAI>().size() == 0);
}

TEST_CASE("registerComponents twice keeps the existing components",
          "[game][components]")
{
    Registry registry;
    registerComponents(registry);
    const Entity entity = registry.spawnEntity();
    registry.addComponent(entity, NetId{7});

    registerComponents(registry);

    REQUIRE(registry.getComponents<NetId>().get(entity)->value == 7);
}

TEST_CASE("a component is not usable before it is registered",
          "[game][components]")
{
    Registry registry;
    const Entity entity = registry.spawnEntity();

    REQUIRE_THROWS_AS(registry.addComponent(entity, Position{}),
                      std::logic_error);
}

TEST_CASE("default values describe a harmless entity", "[game][components]")
{
    REQUIRE(Position{}.value == engine::core::Vec2{});
    REQUIRE(Velocity{}.value == engine::core::Vec2{});
    REQUIRE(Kind{}.owner == NO_PLAYER);
    REQUIRE(PlayerControl{}.keys == 0);
    REQUIRE(Health{}.current == Health{}.max);
    REQUIRE(Lives{}.remaining == GameRules{}.lives);
    REQUIRE(Team{}.side == TeamSide::ENEMIES);
}

TEST_CASE("components of different entities are independent",
          "[game][components]")
{
    Registry registry;
    registerComponents(registry);
    const Entity first = registry.spawnEntity();
    const Entity second = registry.spawnEntity();
    registry.addComponent(first, Health{3, 5});
    registry.addComponent(second, Health{1, 1});

    REQUIRE(registry.getComponents<Health>().get(first)->current == 3);
    REQUIRE(registry.getComponents<Health>().get(second)->current == 1);
}
