/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameInput.cpp
*/

#include "Client/Flow/GameInput.hpp"

#include <catch2/catch_test_macros.hpp>
#include <set>

using client::events::ConnectionFailure;
using client::flow::IGameDevice;
using client::net::ClientSession;
using client::settings::ClientSettings;
using client::settings::ScanCode;

namespace {

class FakeDevice : public IGameDevice {
public:
    [[nodiscard]] bool isKeyDown(ScanCode key) const override
    {
        return held.contains(key);
    }

    void playShot() override
    {
        ++shots;
    }

    void playExplosion() override
    {
        ++explosions;
    }

    [[nodiscard]] engine::core::Vec2
    toScreen(engine::core::Vec2 world) const override
    {
        return world;
    }

    [[nodiscard]] engine::core::Vec2 size() const override
    {
        return {1280.0f, 720.0f};
    }

    std::set<ScanCode> held;
    int shots = 0;
    int explosions = 0;
};

} // namespace

TEST_CASE("the bound keys and the letters move the ship",
          "[client][flow][input]")
{
    FakeDevice device;
    const ClientSettings settings;

    REQUIRE(client::flow::readKeys(device, settings) == 0);
    device.held = {client::settings::KEY_UP, 0x07};
    REQUIRE(client::flow::readKeys(device, settings) ==
            (rtype::protocol::KEY_UP | rtype::protocol::KEY_RIGHT));
}

TEST_CASE("the shoot key and auto-fire both shoot", "[client][flow][input]")
{
    FakeDevice device;
    ClientSettings settings;

    device.held = {client::settings::KEY_SPACE};
    REQUIRE(client::flow::readKeys(device, settings) ==
            rtype::protocol::KEY_SHOOT);
    device.held.clear();
    settings.bindings.autoFire = true;
    REQUIRE(client::flow::readKeys(device, settings) ==
            rtype::protocol::KEY_SHOOT);
}

TEST_CASE("only a session that is not connected has a banner",
          "[client][flow][input]")
{
    REQUIRE(client::flow::bannerOf(ClientSession::State::CONNECTED).empty());
    REQUIRE(client::flow::bannerOf(ClientSession::State::CONNECTING) ==
            "Connecting...");
    REQUIRE(client::flow::bannerOf(ClientSession::State::FAILED) ==
            "Connection lost");
}

TEST_CASE("a refusal keeps the reason of the server, the rest is a timeout",
          "[client][flow][input]")
{
    REQUIRE(client::flow::failureOf(ClientSession::State::REJECTED, 1) ==
            ConnectionFailure::SERVER_FULL);
    REQUIRE(client::flow::failureOf(ClientSession::State::REJECTED, 3) ==
            ConnectionFailure::VERSION);
    REQUIRE(client::flow::failureOf(ClientSession::State::REJECTED, 99) ==
            ConnectionFailure::TIMEOUT);
    REQUIRE(client::flow::failureOf(ClientSession::State::FAILED, 1) ==
            ConnectionFailure::TIMEOUT);
}

TEST_CASE("shots and kills make a sound", "[client][flow][input]")
{
    namespace proto = rtype::protocol;
    FakeDevice device;
    proto::Spawn shot;
    proto::Destroy kill;

    shot.kind = static_cast<std::uint8_t>(proto::EntityKind::PLAYER_MISSILE);
    kill.reason = static_cast<std::uint8_t>(proto::DestroyReason::KILLED);
    client::flow::playSounds(shot, device);
    client::flow::playSounds(kill, device);
    client::flow::playSounds(proto::Destroy{}, device);
    REQUIRE(device.shots == 1);
    REQUIRE(device.explosions == 1);
}
