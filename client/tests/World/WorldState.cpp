/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** WorldState.cpp
*/

#include "Client/World/WorldState.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "FixedPoint.hpp"

using Catch::Approx;
using client::world::WorldState;
namespace proto = rtype::protocol;

namespace {

proto::Welcome welcome(std::uint8_t slot = 0)
{
    proto::Welcome value;

    value.playerIndex = slot;
    value.shipSpeed = 300;
    return value;
}

proto::Spawn ship(std::uint16_t id, std::uint8_t owner, float x, float y)
{
    proto::Spawn spawn;

    spawn.netId = id;
    spawn.kind = static_cast<std::uint8_t>(proto::EntityKind::PLAYER_SHIP);
    spawn.owner = owner;
    spawn.x = proto::toFixed(x);
    spawn.y = proto::toFixed(y);
    return spawn;
}

proto::Snapshot snapshot(std::uint16_t id, float x, float y,
                         std::uint16_t ack = 0)
{
    return proto::Snapshot{
        1, ack, {{id, proto::toFixed(x), proto::toFixed(y)}}};
}

} // namespace

TEST_CASE("a spawn creates an entity and a destroy removes it")
{
    WorldState world;

    world.apply(ship(1, 0, 100, 200), 0.0);
    REQUIRE(world.entityCount() == 1);
    const auto items = world.visible(1.0);

    REQUIRE(items.size() == 1);
    CHECK(items[0].position.x == Approx(100.0F));
    world.apply(proto::Destroy{1, 1, 0}, 0.1);
    CHECK(world.entityCount() == 0);
}

TEST_CASE("other entities are interpolated between two snapshots")
{
    WorldState world;

    world.apply(ship(2, 1, 0, 0), 0.0);
    world.apply(snapshot(2, 0, 0), 1.0);
    world.apply(snapshot(2, 100, 0), 1.1);
    const auto items = world.visible(1.05 + WorldState::INTERPOLATION_DELAY);

    REQUIRE(items.size() == 1);
    CHECK(items[0].position.x == Approx(50.0F).margin(0.1));
}

TEST_CASE("an entity with no news is extrapolated for a limited time")
{
    WorldState world;
    proto::Spawn missile;

    missile.netId = 3;
    missile.kind = static_cast<std::uint8_t>(proto::EntityKind::PLAYER_MISSILE);
    missile.vx = proto::toFixed(100);
    world.apply(missile, 0.0);
    CHECK(world.visible(0.5 + WorldState::INTERPOLATION_DELAY)[0].position.x ==
          Approx(50.0F).margin(0.1));
    CHECK(world.visible(10.0)[0].position.x == Approx(100.0F).margin(0.1));
}

TEST_CASE("an unknown entity in a snapshot is created")
{
    WorldState world;

    world.apply(snapshot(9, 10, 10), 0.0);
    CHECK(world.entityCount() == 1);
}

TEST_CASE("the own ship follows the keys at once")
{
    WorldState world;

    world.configure(welcome(0));
    world.apply(ship(1, 0, 100, 100), 0.0);
    for (std::uint16_t id = 1; id <= 6; ++id) {
        world.predict(id, proto::KEY_RIGHT);
    }
    CHECK(world.visible(0.0)[0].position.x == Approx(130.0F).margin(0.5));
}

TEST_CASE(
    "a snapshot corrects the own ship and replays the unacknowledged keys")
{
    WorldState world;

    world.configure(welcome(0));
    world.apply(ship(1, 0, 100, 100), 0.0);
    for (std::uint16_t id = 1; id <= 6; ++id) {
        world.predict(id, proto::KEY_RIGHT);
    }
    world.apply(snapshot(1, 110, 100, 4), 0.1);
    CHECK(world.visible(0.1)[0].position.x == Approx(120.0F).margin(0.5));
    world.apply(snapshot(1, 130, 100, 6), 0.2);
    CHECK(world.visible(0.2)[0].position.x == Approx(130.0F).margin(0.5));
}

TEST_CASE("the server wins when it disagrees with the prediction")
{
    WorldState world;

    world.configure(welcome(0));
    world.apply(ship(1, 0, 100, 100), 0.0);
    world.predict(1, proto::KEY_RIGHT);
    world.apply(snapshot(1, 50, 60, 1), 0.1);
    const auto position = world.visible(0.1)[0].position;

    CHECK(position.x == Approx(50.0F));
    CHECK(position.y == Approx(60.0F));
}

TEST_CASE("another player's ship is not predicted")
{
    WorldState world;

    world.configure(welcome(0));
    world.apply(ship(2, 1, 100, 100), 0.0);
    world.predict(1, proto::KEY_RIGHT);
    CHECK(world.visible(0.0)[0].position.x == Approx(100.0F));
}

TEST_CASE("entities not updated for a while are forgotten, the own ship stays")
{
    WorldState world;

    world.configure(welcome(0));
    world.apply(ship(1, 0, 0, 0), 0.0);
    world.apply(ship(2, 1, 0, 0), 0.0);
    world.update(WorldState::STALE_AFTER + 0.5);
    CHECK(world.entityCount() == 1);
    CHECK(world.visible(3.0)[0].owner == 0);
}

TEST_CASE("players, scores and the game state are tracked")
{
    WorldState world;

    world.apply(proto::PlayerJoined{1, "Eve"}, 0.0);
    world.apply(proto::Score{1, 250}, 0.0);
    world.apply(proto::GameState{2, 3}, 0.0);
    CHECK(world.players()[1].present);
    CHECK(world.players()[1].name == "Eve");
    CHECK(world.players()[1].score == 250);
    CHECK(world.phase() == 2);
    CHECK(world.level() == 3);
    world.apply(proto::PlayerLeft{1, 0}, 0.0);
    CHECK_FALSE(world.players()[1].present);
}

TEST_CASE("the own ship loses a life when it is killed, not when it expires")
{
    WorldState world;

    world.configure(welcome(0));
    CHECK(world.lives() == world.rules().lives);
    world.apply(ship(1, 0, 0, 0), 0.0);
    world.apply(
        proto::Destroy{
            1, static_cast<std::uint8_t>(proto::DestroyReason::EXPIRED), 0},
        0.1);
    CHECK(world.lives() == world.rules().lives);
    world.apply(ship(1, 0, 0, 0), 0.2);
    world.apply(
        proto::Destroy{
            1, static_cast<std::uint8_t>(proto::DestroyReason::KILLED), 0},
        0.3);
    CHECK(world.lives() + 1 == world.rules().lives);
}

TEST_CASE("the death of another ship does not cost a life")
{
    WorldState world;

    world.configure(welcome(0));
    world.apply(ship(2, 1, 0, 0), 0.0);
    world.apply(
        proto::Destroy{
            2, static_cast<std::uint8_t>(proto::DestroyReason::KILLED), 0},
        0.1);
    CHECK(world.lives() == world.rules().lives);
}
