/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudFeed.cpp
*/

#include "Client/Hud/HudFeed.hpp"

#include <catch2/catch_test_macros.hpp>

using client::events::CueDirection;
using client::hud::HudFeed;
namespace proto = rtype::protocol;

namespace {

constexpr std::uint8_t SHIP = 1;
constexpr std::uint8_t BYDOS = 2;
constexpr std::uint8_t MISSILE = 3;
constexpr std::uint8_t KILLED = 1;

proto::Spawn spawnOf(std::uint16_t id, std::uint8_t kind, std::uint8_t owner,
                     std::int16_t x)
{
    proto::Spawn spawn;

    spawn.netId = id;
    spawn.kind = kind;
    spawn.owner = owner;
    spawn.x = x;
    return spawn;
}

proto::Destroy destroyOf(std::uint16_t id, std::uint8_t reason)
{
    proto::Destroy destroy;

    destroy.netId = id;
    destroy.reason = reason;
    return destroy;
}

void join(HudFeed &feed)
{
    proto::Welcome welcome;

    welcome.playerIndex = 1;
    feed.welcome(welcome);
    feed.feed(spawnOf(10, SHIP, 1, 100));
}

} // namespace

TEST_CASE("score, stage and lives reach the hud state", "[client][hud][feed]")
{
    HudFeed feed;

    join(feed);
    feed.feed(proto::Score{1, 1234});
    feed.feed(proto::Score{2, 99});
    feed.feed(proto::GameState{1, 3});
    feed.feed(destroyOf(10, KILLED));

    REQUIRE(feed.state().score == 1234);
    REQUIRE(feed.state().level == 3);
    REQUIRE(feed.state().lives == client::hud::STARTING_LIVES - 1);
}

TEST_CASE("a ship removed without a kill keeps its life", "[client][hud][feed]")
{
    HudFeed feed;

    join(feed);
    feed.feed(destroyOf(10, 0));

    REQUIRE(feed.state().lives == client::hud::STARTING_LIVES);
}

TEST_CASE("a shot of another player shows a cue on its side",
          "[client][hud][feed]")
{
    HudFeed feed;

    join(feed);
    feed.feed(spawnOf(20, MISSILE, 2, 1200));
    feed.feed(spawnOf(21, MISSILE, 1, 100));

    REQUIRE(feed.cue(CueDirection::RIGHT) == "Shot >>");
    REQUIRE(feed.cue(CueDirection::LEFT).empty());
}

TEST_CASE("a kill shows an explosion where the entity was last seen",
          "[client][hud][feed]")
{
    HudFeed feed;

    join(feed);
    feed.feed(spawnOf(30, BYDOS, proto::NO_PLAYER, 1200));
    proto::Snapshot snapshot;
    snapshot.entities.push_back({30, 200, 50});
    feed.feed(snapshot);
    feed.feed(destroyOf(30, KILLED));

    REQUIRE(feed.cue(CueDirection::LEFT) == "<< Explosion");
}

TEST_CASE("a cue disappears after its duration", "[client][hud][feed]")
{
    HudFeed feed;

    join(feed);
    feed.feed(spawnOf(20, MISSILE, 2, 1200));
    feed.update(5.0f, false);

    REQUIRE(feed.cue(CueDirection::RIGHT).empty());
}

TEST_CASE("invalid kinds and phases are ignored", "[client][hud][feed]")
{
    HudFeed feed;

    join(feed);
    feed.feed(spawnOf(40, 200, 2, 1200));
    feed.feed(proto::GameState{77, 9});
    feed.feed(destroyOf(40, 99));

    REQUIRE(feed.state().level == 1);
    REQUIRE(feed.cue(CueDirection::RIGHT).empty());
}
