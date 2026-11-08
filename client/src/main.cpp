/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** main.cpp
*/

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "Client/Net/ClientSession.hpp"
#include "Client/Render/GameWindow.hpp"
#include "Client/Scenes/Starfield.hpp"
#include "Client/Settings/SettingsStore.hpp"
#include "Client/World/WorldState.hpp"

namespace {

using client::net::ClientSession;
using client::settings::GameAction;
using rtype::protocol::Clock;

struct Options {
    std::string host = "127.0.0.1";
    std::uint16_t port = 4242;
    std::string name = "Player";
    bool help = false;
};

void printUsage()
{
    std::cout << "Usage: r-type_client [HOST [PORT]] [--host HOST] "
                 "[--port PORT] [--name NAME]\n"
                 "  Defaults: 127.0.0.1, 4242, Player. Arrow keys move, space "
                 "shoots,\n  Escape quits. Keys and display can be changed in "
                 "the settings file.\n";
}

bool parsePort(const std::string &text, std::uint16_t &port)
{
    const int value = std::atoi(text.c_str());

    if (value < 1 || value > 65535) {
        return false;
    }
    port = static_cast<std::uint16_t>(value);
    return true;
}

bool parseOptions(int argc, char **argv, Options &options)
{
    int positional = 0;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const bool hasValue = i + 1 < argc;

        if (arg == "--help" || arg == "-h") {
            options.help = true;
        } else if (arg == "--host" && hasValue) {
            options.host = argv[++i];
        } else if (arg == "--name" && hasValue) {
            options.name = argv[++i];
        } else if (arg == "--port" && hasValue) {
            if (!parsePort(argv[++i], options.port)) {
                return false;
            }
        } else if (arg.rfind("--", 0) != 0 && positional < 2) {
            const bool isPort = positional == 1;

            if (isPort && !parsePort(arg, options.port)) {
                return false;
            }
            options.host = isPort ? options.host : arg;
            ++positional;
        } else {
            return false;
        }
    }
    return true;
}

std::filesystem::path settingsPath()
{
    const char *home = std::getenv("HOME");

    if (home == nullptr) {
        home = std::getenv("USERPROFILE");
    }
    if (home == nullptr) {
        return "rtype-settings.cfg";
    }
    return std::filesystem::path(home) / ".config" / "r-type" / "settings.cfg";
}

std::string findAssets(const char *program)
{
    const std::filesystem::path exeDir =
        std::filesystem::path(program).parent_path();
    std::vector<std::filesystem::path> roots = {
        "assets", "../assets", "../../assets", "../../../assets"};

    for (const char *up : {"", "..", "../..", "../../.."}) {
        roots.push_back(exeDir / up / "assets");
    }
    for (const auto &root : roots) {
        if (std::filesystem::exists(root / "sprites/r-typesheet42.gif")) {
            return root.string();
        }
    }
    return "assets";
}

bool held(const client::render::GameWindow &window,
          const client::settings::KeyBindings &bindings, GameAction action,
          client::settings::ScanCode alternative)
{
    return window.isKeyDown(
               client::settings::bindingOf(bindings, action).key) ||
           window.isKeyDown(alternative);
}

std::uint8_t readKeys(const client::render::GameWindow &window,
                      const client::settings::ClientSettings &settings)
{
    constexpr client::settings::ScanCode KEY_W = 0x1A;
    constexpr client::settings::ScanCode KEY_A = 0x04;
    constexpr client::settings::ScanCode KEY_S = 0x16;
    constexpr client::settings::ScanCode KEY_D = 0x07;
    const auto &bindings = settings.bindings;
    std::uint8_t keys = 0;

    keys |= held(window, bindings, GameAction::MOVE_UP, KEY_W)
                ? rtype::protocol::KEY_UP
                : 0;
    keys |= held(window, bindings, GameAction::MOVE_DOWN, KEY_S)
                ? rtype::protocol::KEY_DOWN
                : 0;
    keys |= held(window, bindings, GameAction::MOVE_LEFT, KEY_A)
                ? rtype::protocol::KEY_LEFT
                : 0;
    keys |= held(window, bindings, GameAction::MOVE_RIGHT, KEY_D)
                ? rtype::protocol::KEY_RIGHT
                : 0;
    const bool shoot =
        bindings.autoFire ||
        window.isKeyDown(
            client::settings::bindingOf(bindings, GameAction::SHOOT).key);

    return keys | (shoot ? rtype::protocol::KEY_SHOOT : 0);
}

std::string bannerOf(const ClientSession &session,
                     const client::world::WorldState &world)
{
    switch (session.state()) {
        case ClientSession::State::CONNECTING:
            return "Connecting...";
        case ClientSession::State::REJECTED:
            return "Server refused the connection";
        case ClientSession::State::FAILED:
            return "Connection lost";
        case ClientSession::State::CLOSED:
            return "Disconnected";
        case ClientSession::State::CONNECTED:
            break;
    }
    if (world.phase() ==
        static_cast<std::uint8_t>(rtype::protocol::GamePhase::GAME_OVER)) {
        return "Game over";
    }
    return {};
}

void playSounds(const rtype::protocol::Message &message,
                client::render::GameWindow &window)
{
    namespace proto = rtype::protocol;

    if (const auto *spawn = std::get_if<proto::Spawn>(&message)) {
        if (spawn->kind ==
            static_cast<std::uint8_t>(proto::EntityKind::PLAYER_MISSILE)) {
            window.playShot();
        }
    } else if (const auto *destroy = std::get_if<proto::Destroy>(&message)) {
        if (destroy->reason ==
            static_cast<std::uint8_t>(proto::DestroyReason::KILLED)) {
            window.playExplosion();
        }
    }
}

double seconds(Clock::time_point start, Clock::time_point now)
{
    return std::chrono::duration<double>(now - start).count();
}

void runGame(ClientSession &session, client::render::GameWindow &window,
             const client::settings::ClientSettings &settings)
{
    const auto start = Clock::now();
    client::world::WorldState world;
    client::scenes::Starfield stars(120, 7);
    double accumulator = 0.0;
    double previous = 0.0;

    while (window.isOpen()) {
        const auto tick = Clock::now();
        const double now = seconds(start, tick);

        window.pollEvents();
        if (window.isKeyDown(client::settings::bindingOf(settings.bindings,
                                                         GameAction::OPEN_MENU)
                                 .key)) {
            window.close();
            break;
        }
        session.update(tick);
        if (!world.configured() && session.welcome()) {
            world.configure(*session.welcome());
        }
        for (const auto &message : session.takeMessages()) {
            world.apply(message, now);
            playSounds(message, window);
        }
        accumulator += std::min(now - previous, 0.25);
        previous = now;
        const double step = 1.0 / static_cast<double>(world.rules().tickRate);

        while (accumulator >= step && world.configured()) {
            const std::uint8_t keys = readKeys(window, settings);

            world.predict(session.sendInput(keys, tick), keys);
            accumulator -= step;
        }
        stars.advance(static_cast<float>(step));
        world.update(now);
        window.draw({world, stars, now, bannerOf(session, world),
                     static_cast<int>(session.rtt().count()), settings.palette,
                     settings.highContrast});
    }
    session.disconnect(Clock::now());
}

} // namespace

int main(int argc, char **argv)
{
    Options options;

    if (!parseOptions(argc, argv, options)) {
        printUsage();
        return 1;
    }
    if (options.help) {
        printUsage();
        return 0;
    }
    client::settings::SettingsStore store(settingsPath());

    store.load();
    ClientSession session(options.host, options.port, options.name);
    client::render::GameWindow window(store.settings(), "R-Type",
                                      findAssets(argv[0]));

    runGame(session, window, store.settings());
    return 0;
}
