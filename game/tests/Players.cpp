/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Players.cpp
*/

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <variant>

#include "Rtype/ShipMovement.hpp"
#include "Rtype/Simulation.hpp"

using Catch::Approx;
using engine::core::Vec2;
using rtype::game::DestroyEvent;
using rtype::game::DestroyReason;
using rtype::game::EntityKind;
using rtype::game::EntityState;
using rtype::game::GameEvent;
using rtype::game::GameRules;
using rtype::game::moveShip;
using rtype::game::NO_PLAYER;
using rtype::game::Simulation;
using rtype::game::SpawnEvent;
namespace keys = rtype::game::keys;

namespace {

GameRules quiet()
{
    GameRules rules;

    rules.enemySpawnInterval = 0.0F;
    return rules;
}

constexpr std::uint32_t SEED = 7;
constexpr std::uint8_t SLOT = 1;
constexpr int ONE_SECOND = 60;

void stepTimes(Simulation &simulation, int count)
{
    for (int i = 0; i < count; ++i) {
        simulation.step();
    }
}

std::vector<EntityState> ofKind(const Simulation &simulation, EntityKind kind)
{
    std::vector<EntityState> found;

    for (const EntityState &state : simulation.states()) {
        if (state.kind == kind) {
            found.push_back(state);
        }
    }
    return found;
}

Vec2 shipPosition(const Simulation &simulation)
{
    return ofKind(simulation, EntityKind::PLAYER_SHIP).front().position;
}

template <typename Event>
std::vector<Event> eventsOf(const std::vector<GameEvent> &events)
{
    std::vector<Event> found;

    for (const GameEvent &event : events) {
        if (const auto *typed = std::get_if<Event>(&event)) {
            found.push_back(*typed);
        }
    }
    return found;
}

Simulation withPlayer()
{
    Simulation simulation(quiet(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));
    return simulation;
}

} // namespace

TEST_CASE("adding a player spawns a ship and tells it", "[game][players]")
{
    Simulation simulation(quiet(), SEED);

    REQUIRE(simulation.addPlayer(SLOT));

    const auto spawns = eventsOf<SpawnEvent>(simulation.takeEvents());
    const auto ships = ofKind(simulation, EntityKind::PLAYER_SHIP);

    REQUIRE(spawns.size() == 1);
    REQUIRE(spawns[0].kind == EntityKind::PLAYER_SHIP);
    REQUIRE(spawns[0].owner == SLOT);
    REQUIRE(ships.size() == 1);
    REQUIRE(ships[0].netId == spawns[0].netId);
    REQUIRE(ships[0].position == spawns[0].position);
}

TEST_CASE("a slot can only be taken once", "[game][players]")
{
    Simulation simulation = withPlayer();

    REQUIRE_FALSE(simulation.addPlayer(SLOT));
    REQUIRE(simulation.states().size() == 1);
    REQUIRE(simulation.addPlayer(0));
    REQUIRE(simulation.states().size() == 2);
}

TEST_CASE("a slot beyond maxPlayers is refused", "[game][players]")
{
    GameRules rules = quiet();
    rules.maxPlayers = 2;
    Simulation simulation(rules, SEED);

    REQUIRE(simulation.addPlayer(1));
    REQUIRE_FALSE(simulation.addPlayer(2));
    REQUIRE_FALSE(simulation.addPlayer(NO_PLAYER));
    REQUIRE(simulation.takeEvents().size() == 1);
}

TEST_CASE("two players get different ships and net ids", "[game][players]")
{
    Simulation simulation(quiet(), SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(simulation.addPlayer(1));

    const auto ships = ofKind(simulation, EntityKind::PLAYER_SHIP);

    REQUIRE(ships.size() == 2);
    REQUIRE(ships[0].netId != ships[1].netId);
    REQUIRE(ships[0].position != ships[1].position);
}

TEST_CASE("removing a player destroys the ship and tells it", "[game][players]")
{
    Simulation simulation = withPlayer();
    const auto spawn = eventsOf<SpawnEvent>(simulation.takeEvents()).front();

    simulation.removePlayer(SLOT);

    const auto destroys = eventsOf<DestroyEvent>(simulation.takeEvents());

    REQUIRE(destroys.size() == 1);
    REQUIRE(destroys[0].netId == spawn.netId);
    REQUIRE(destroys[0].reason == DestroyReason::LEFT_SCREEN);
    REQUIRE(destroys[0].killer == NO_PLAYER);
    REQUIRE(simulation.states().empty());
}

TEST_CASE("removing a player twice is safe", "[game][players]")
{
    Simulation simulation = withPlayer();

    simulation.removePlayer(SLOT);
    static_cast<void>(simulation.takeEvents());
    simulation.removePlayer(SLOT);
    simulation.removePlayer(0);
    simulation.removePlayer(NO_PLAYER);

    REQUIRE(simulation.takeEvents().empty());
}

TEST_CASE("a slot can be taken again after its player left", "[game][players]")
{
    Simulation simulation = withPlayer();
    const auto first = simulation.states().front().netId;

    simulation.removePlayer(SLOT);
    REQUIRE(simulation.addPlayer(SLOT));

    REQUIRE(simulation.states().size() == 1);
    REQUIRE(simulation.states().front().netId != first);
}

TEST_CASE("events are given once, in the order they happened",
          "[game][players]")
{
    Simulation simulation(quiet(), SEED);

    REQUIRE(simulation.addPlayer(0));
    simulation.removePlayer(0);

    const auto events = simulation.takeEvents();

    REQUIRE(events.size() == 2);
    REQUIRE(std::holds_alternative<SpawnEvent>(events[0]));
    REQUIRE(std::holds_alternative<DestroyEvent>(events[1]));
    REQUIRE(simulation.takeEvents().empty());
}

TEST_CASE("currentWorld describes every living entity", "[game][players]")
{
    Simulation simulation(quiet(), SEED);

    REQUIRE(simulation.addPlayer(0));
    REQUIRE(simulation.addPlayer(2));

    const auto world = simulation.currentWorld();
    const auto events = eventsOf<SpawnEvent>(simulation.takeEvents());

    REQUIRE(world.size() == 2);
    REQUIRE(world[0].netId == events[0].netId);
    REQUIRE(world[0].owner == 0);
    REQUIRE(world[1].netId == events[1].netId);
    REQUIRE(world[1].owner == 2);
}

TEST_CASE("a ship moves in the direction of the held keys", "[game][players]")
{
    Simulation simulation = withPlayer();
    const Vec2 start = shipPosition(simulation);
    const float speed = simulation.rules().shipSpeed;

    simulation.setInput(SLOT, keys::RIGHT);
    stepTimes(simulation, ONE_SECOND);

    REQUIRE(shipPosition(simulation).x == Approx(start.x + speed).margin(0.1));
    REQUIRE(shipPosition(simulation).y == Approx(start.y));
}

TEST_CASE("up moves the ship up the screen", "[game][players]")
{
    Simulation simulation = withPlayer();
    const Vec2 start = shipPosition(simulation);
    const float half = simulation.rules().shipSpeed / 2.0F;

    simulation.setInput(SLOT, keys::UP);
    stepTimes(simulation, ONE_SECOND / 2);

    REQUIRE(shipPosition(simulation).y == Approx(start.y - half).margin(0.1));
}

TEST_CASE("a diagonal covers the same distance as a straight line",
          "[game][players]")
{
    Simulation simulation = withPlayer();
    const Vec2 start = shipPosition(simulation);

    simulation.setInput(SLOT, keys::DOWN | keys::RIGHT);
    stepTimes(simulation, ONE_SECOND / 2);

    const Vec2 moved = shipPosition(simulation) - start;

    REQUIRE(moved.x > 0.0F);
    REQUIRE(moved.x == Approx(moved.y).margin(0.01));
    REQUIRE(engine::core::length(moved) ==
            Approx(simulation.rules().shipSpeed / 2.0F).margin(0.1));
}

TEST_CASE("opposite keys leave the ship where it is", "[game][players]")
{
    Simulation simulation = withPlayer();
    const Vec2 start = shipPosition(simulation);

    simulation.setInput(SLOT, keys::LEFT | keys::RIGHT);
    stepTimes(simulation, 10);

    REQUIRE(shipPosition(simulation) == start);
}

TEST_CASE("the keys are kept until they change", "[game][players]")
{
    Simulation simulation = withPlayer();
    const Vec2 start = shipPosition(simulation);
    const float perStep =
        simulation.rules().shipSpeed / static_cast<float>(ONE_SECOND);

    simulation.setInput(SLOT, keys::RIGHT);
    stepTimes(simulation, 10);
    stepTimes(simulation, 10);
    REQUIRE(shipPosition(simulation).x ==
            Approx(start.x + 20.0F * perStep).margin(0.01));

    simulation.setInput(SLOT, 0);
    stepTimes(simulation, 10);
    REQUIRE(shipPosition(simulation).x ==
            Approx(start.x + 20.0F * perStep).margin(0.01));

    simulation.setInput(SLOT, keys::LEFT);
    stepTimes(simulation, 20);
    REQUIRE(shipPosition(simulation).x == Approx(start.x).margin(0.01));
}

TEST_CASE("unknown bits and unknown players are ignored", "[game][players]")
{
    Simulation simulation = withPlayer();
    const Vec2 start = shipPosition(simulation);

    simulation.setInput(0, keys::RIGHT);
    simulation.setInput(NO_PLAYER, keys::RIGHT);
    simulation.setInput(SLOT, 0xE0);
    stepTimes(simulation, 10);
    REQUIRE(shipPosition(simulation) == start);

    simulation.setInput(SLOT, 0xE0 | keys::RIGHT);
    stepTimes(simulation, 10);
    REQUIRE(shipPosition(simulation).x > start.x);
}

TEST_CASE("a ship stays inside the world", "[game][players]")
{
    Simulation simulation = withPlayer();
    const GameRules rules = simulation.rules();

    simulation.setInput(SLOT, keys::LEFT | keys::UP);
    stepTimes(simulation, 10 * ONE_SECOND);
    REQUIRE(shipPosition(simulation) == Vec2{0.0F, 0.0F});

    simulation.setInput(SLOT, keys::RIGHT | keys::DOWN);
    stepTimes(simulation, 10 * ONE_SECOND);
    REQUIRE(shipPosition(simulation) ==
            Vec2{rules.worldWidth, rules.worldHeight});
}

TEST_CASE("the server and a predicting client move a ship the same way",
          "[game][players]")
{
    Simulation simulation = withPlayer();
    const GameRules rules = simulation.rules();
    const float step = static_cast<float>(1.0 / rules.tickRate);
    const std::uint8_t sequence[] = {keys::RIGHT,
                                     keys::RIGHT | keys::UP,
                                     keys::UP,
                                     0,
                                     keys::LEFT | keys::DOWN,
                                     keys::DOWN,
                                     keys::RIGHT | keys::SHOOT};
    Vec2 predicted = shipPosition(simulation);

    for (int i = 0; i < 70; ++i) {
        const std::uint8_t pressed = sequence[i % 7] & ~keys::SHOOT;
        simulation.setInput(SLOT, pressed);
        simulation.step();
        predicted = moveShip(predicted, pressed, rules, step);
        REQUIRE(shipPosition(simulation) == predicted);
    }
}

TEST_CASE("a ship that does not shoot fires nothing", "[game][shooting]")
{
    Simulation simulation = withPlayer();

    simulation.setInput(SLOT, keys::RIGHT);
    stepTimes(simulation, ONE_SECOND);

    REQUIRE(ofKind(simulation, EntityKind::PLAYER_MISSILE).empty());
}

TEST_CASE("shooting spawns a missile in front of the ship", "[game][shooting]")
{
    Simulation simulation = withPlayer();
    static_cast<void>(simulation.takeEvents());

    simulation.setInput(SLOT, keys::SHOOT);
    simulation.step();

    const auto spawns = eventsOf<SpawnEvent>(simulation.takeEvents());
    const auto missiles = ofKind(simulation, EntityKind::PLAYER_MISSILE);

    REQUIRE(spawns.size() == 1);
    REQUIRE(spawns[0].kind == EntityKind::PLAYER_MISSILE);
    REQUIRE(spawns[0].owner == SLOT);
    REQUIRE(spawns[0].velocity.x == simulation.rules().missileSpeed);
    REQUIRE(spawns[0].position.x > shipPosition(simulation).x);
    REQUIRE(missiles.size() == 1);
    REQUIRE(missiles[0].netId == spawns[0].netId);
    REQUIRE(missiles[0].netId !=
            ofKind(simulation, EntityKind::PLAYER_SHIP).front().netId);
}

TEST_CASE("a missile flies to the right", "[game][shooting]")
{
    Simulation simulation = withPlayer();

    simulation.setInput(SLOT, keys::SHOOT);
    simulation.step();
    simulation.setInput(SLOT, 0);

    const float before =
        ofKind(simulation, EntityKind::PLAYER_MISSILE).front().position.x;
    stepTimes(simulation, 10);
    const float after =
        ofKind(simulation, EntityKind::PLAYER_MISSILE).front().position.x;

    REQUIRE(after - before ==
            Approx(10.0F * simulation.rules().missileSpeed / ONE_SECOND)
                .margin(0.01));
}

TEST_CASE("the fire cooldown limits the rate of fire", "[game][shooting]")
{
    Simulation simulation = withPlayer();

    simulation.setInput(SLOT, keys::SHOOT);
    simulation.step();
    simulation.step();
    REQUIRE(ofKind(simulation, EntityKind::PLAYER_MISSILE).size() == 1);

    stepTimes(simulation, ONE_SECOND - 2);
    REQUIRE(ofKind(simulation, EntityKind::PLAYER_MISSILE).size() == 4);
}

TEST_CASE("a ship can shoot again once the cooldown is over",
          "[game][shooting]")
{
    Simulation simulation = withPlayer();

    simulation.setInput(SLOT, keys::SHOOT);
    simulation.step();
    simulation.setInput(SLOT, 0);
    stepTimes(simulation, ONE_SECOND);
    simulation.setInput(SLOT, keys::SHOOT);
    simulation.step();

    REQUIRE(ofKind(simulation, EntityKind::PLAYER_MISSILE).size() == 2);
}

TEST_CASE("two simulations with the same inputs stay identical",
          "[game][players]")
{
    Simulation first = withPlayer();
    Simulation second = withPlayer();

    for (int i = 0; i < 100; ++i) {
        const std::uint8_t pressed =
            (i % 3 == 0) ? keys::SHOOT | keys::UP : keys::RIGHT | keys::DOWN;
        first.setInput(SLOT, pressed);
        second.setInput(SLOT, pressed);
        first.step();
        second.step();
    }
    const auto a = first.states();
    const auto b = second.states();

    REQUIRE(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        REQUIRE(a[i].netId == b[i].netId);
        REQUIRE(a[i].position == b[i].position);
    }
    REQUIRE(first.takeEvents().size() == second.takeEvents().size());
}
