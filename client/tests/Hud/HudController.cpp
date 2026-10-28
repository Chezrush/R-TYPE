/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudController.cpp
*/

#include "Client/Hud/HudController.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>

#include "Client/Events/GameEvents.hpp"
#include "Engine/Core/EventBus.hpp"

using client::events::DestroyReason;
using client::events::EntityDestroyed;
using client::events::EntityKind;
using client::events::EntitySpawned;
using client::events::GamePhase;
using client::events::GameStateChanged;
using client::events::PlayerJoined;
using client::events::PlayerLeft;
using client::events::ScoreChanged;
using client::events::WelcomeReceived;
using client::hud::HudController;
using client::hud::STARTING_LIVES;
using engine::core::EventBus;

namespace {

void joinAs(EventBus &bus, std::uint8_t player, std::uint16_t ship)
{
    bus.publish(WelcomeReceived{player, 4});
    bus.publish(EntitySpawned{ship, EntityKind::PLAYER_SHIP, player});
}

void kill(EventBus &bus, std::uint16_t ship)
{
    bus.publish(EntityDestroyed{ship, DestroyReason::KILLED, 1});
}

} // namespace

TEST_CASE("welcome makes us the local player, without a ship yet",
          "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);

    bus.publish(WelcomeReceived{1, 4});

    REQUIRE(hud.state().localPlayer == 1);
    REQUIRE(hud.state().connected[1]);
    REQUIRE_FALSE(hud.state().localShip.has_value());
    REQUIRE_FALSE(hud.state().localAlive);
}

TEST_CASE("our ship is the player ship spawned with our index",
          "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);
    bus.publish(WelcomeReceived{1, 4});

    bus.publish(EntitySpawned{7, EntityKind::PLAYER_SHIP, 0});
    bus.publish(EntitySpawned{8, EntityKind::PLAYER_MISSILE, 1});
    REQUIRE_FALSE(hud.state().localAlive);

    bus.publish(EntitySpawned{9, EntityKind::PLAYER_SHIP, 1});
    REQUIRE(hud.state().localAlive);
    REQUIRE(hud.state().localShip == std::uint16_t{9});
}

TEST_CASE("a killed ship costs one life", "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);
    joinAs(bus, 0, 42);

    kill(bus, 7);
    REQUIRE(hud.state().lives == STARTING_LIVES);

    kill(bus, 42);
    kill(bus, 42);
    REQUIRE(hud.state().lives == STARTING_LIVES - 1);
    REQUIRE_FALSE(hud.state().localAlive);
}

TEST_CASE("our ship removed for another reason costs no life",
          "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);
    joinAs(bus, 0, 42);

    bus.publish(EntityDestroyed{42, DestroyReason::GONE, 0xFF});

    REQUIRE(hud.state().lives == STARTING_LIVES);
    REQUIRE_FALSE(hud.state().localAlive);
}

TEST_CASE("respawn is a new spawn of our ship", "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);
    joinAs(bus, 0, 42);
    kill(bus, 42);

    bus.publish(EntitySpawned{43, EntityKind::PLAYER_SHIP, 0});
    REQUIRE(hud.state().localAlive);
    REQUIRE(hud.state().localShip == std::uint16_t{43});
    REQUIRE(hud.state().lives == STARTING_LIVES - 1);
}

TEST_CASE("a reused net_id does not kill us twice", "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);
    joinAs(bus, 0, 42);
    kill(bus, 42);

    bus.publish(EntitySpawned{42, EntityKind::BYDOS, 0xFF});
    kill(bus, 42);
    REQUIRE(hud.state().lives == STARTING_LIVES - 1);
}

TEST_CASE("lives never go below zero", "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);
    bus.publish(WelcomeReceived{0, 4});

    for (std::uint16_t id = 1; id <= 5; ++id) {
        bus.publish(EntitySpawned{id, EntityKind::PLAYER_SHIP, 0});
        kill(bus, id);
    }
    REQUIRE(hud.state().lives == 0);
}

TEST_CASE("only our own score is shown", "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);

    bus.publish(ScoreChanged{0, 500});
    REQUIRE(hud.state().score == 0);

    bus.publish(WelcomeReceived{0, 4});
    bus.publish(ScoreChanged{1, 900});
    bus.publish(ScoreChanged{0, 79500});
    REQUIRE(hud.state().score == 79500);
}

TEST_CASE("players join and leave", "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);

    bus.publish(PlayerJoined{2, "Ripley"});
    bus.publish(EntitySpawned{11, EntityKind::BYDOS, 3});
    REQUIRE(hud.state().connected[2]);
    REQUIRE_FALSE(hud.state().connected[3]);

    bus.publish(PlayerLeft{2, 1});
    REQUIRE_FALSE(hud.state().connected[2]);
}

TEST_CASE("out of range player indexes are ignored",
          "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);

    bus.publish(WelcomeReceived{9, 16});
    bus.publish(PlayerJoined{4, "Five"});
    bus.publish(EntitySpawned{2, EntityKind::PLAYER_SHIP, 0xFF});
    bus.publish(PlayerLeft{200, 0});

    REQUIRE_FALSE(hud.state().localPlayer.has_value());
    REQUIRE(hud.state().connected == std::array<bool, 4>{});
}

TEST_CASE("the beam follows the shoot key while alive",
          "[client][hud][controller]")
{
    EventBus bus;
    HudController hud(bus, 1.0f);

    hud.update(0.5f, true);
    REQUIRE(hud.state().beamCharge == 0.0f);

    joinAs(bus, 0, 1);
    hud.update(0.5f, true);
    REQUIRE(hud.state().beamCharge == 0.5f);
    hud.update(0.016f, false);
    REQUIRE(hud.state().beamCharge == 0.0f);

    hud.update(0.5f, true);
    kill(bus, 1);
    REQUIRE(hud.state().beamCharge == 0.0f);
}

TEST_CASE("level and phase follow game_state", "[client][hud][controller]")
{
    EventBus bus;
    const HudController hud(bus);

    bus.publish(GameStateChanged{GamePhase::GAME_OVER, 3});

    REQUIRE(hud.state().level == 3);
    REQUIRE(hud.state().phase == GamePhase::GAME_OVER);
}

TEST_CASE("a destroyed controller stops listening", "[client][hud][controller]")
{
    EventBus bus;
    const std::size_t before = bus.size();
    {
        const HudController hud(bus);
        REQUIRE(bus.size() > before);
    }
    REQUIRE(bus.size() == before);
    bus.publish(WelcomeReceived{0, 4});
}
