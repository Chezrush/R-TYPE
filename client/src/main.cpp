/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** main.cpp
*/

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "Client/Hud/HudFeed.hpp"
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
    unsigned value = 0;
    const char *end = std::to_address(text.end());
    const auto [last, error] = std::from_chars(text.data(), end, value);

    if (error != std::errc() || last != end || value < 1 || value > 65535) {
        return false;
    }
    port = static_cast<std::uint16_t>(value);
    return true;
}

bool parseValue(const std::string &flag, const std::string &value,
                Options &options)
{
    if (flag == "--host") {
        options.host = value;
        return true;
    }
    if (flag == "--name") {
        options.name = value;
        return true;
    }
    return flag == "--port" && parsePort(value, options.port);
}

bool parsePositional(const std::string &arg, int &positional, Options &options)
{
    if (arg.rfind("--", 0) == 0 || positional >= 2) {
        return false;
    }
    if (positional == 0) {
        options.host = arg;
    } else if (!parsePort(arg, options.port)) {
        return false;
    }
    ++positional;
    return true;
}

bool parseOptions(const std::vector<std::string> &args, Options &options)
{
    int positional = 0;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const bool hasValue = i + 1 < args.size();

        if (args[i] == "--help" || args[i] == "-h") {
            options.help = true;
        } else if (hasValue && parseValue(args[i], args[i + 1], options)) {
            ++i;
        } else if (!parsePositional(args[i], positional, options)) {
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

struct MoveKey {
    GameAction action;
    client::settings::ScanCode alternative;
    unsigned bit;
};

constexpr client::settings::ScanCode KEY_W = 0x1A;
constexpr client::settings::ScanCode KEY_A = 0x04;
constexpr client::settings::ScanCode KEY_S = 0x16;
constexpr client::settings::ScanCode KEY_D = 0x07;

const std::array<MoveKey, 4> MOVES = {{
    {GameAction::MOVE_UP, KEY_W, rtype::protocol::KEY_UP},
    {GameAction::MOVE_DOWN, KEY_S, rtype::protocol::KEY_DOWN},
    {GameAction::MOVE_LEFT, KEY_A, rtype::protocol::KEY_LEFT},
    {GameAction::MOVE_RIGHT, KEY_D, rtype::protocol::KEY_RIGHT},
}};

std::uint8_t readKeys(const client::render::GameWindow &window,
                      const client::settings::ClientSettings &settings)
{
    const auto &bindings = settings.bindings;
    const auto shootKey =
        client::settings::bindingOf(bindings, GameAction::SHOOT).key;
    unsigned keys = 0;

    for (const MoveKey &move : MOVES) {
        if (held(window, bindings, move.action, move.alternative)) {
            keys |= move.bit;
        }
    }
    if (bindings.autoFire || window.isKeyDown(shootKey)) {
        keys |= rtype::protocol::KEY_SHOOT;
    }
    return static_cast<std::uint8_t>(keys);
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

struct Game {
    ClientSession &session;
    client::render::GameWindow &window;
    const client::settings::ClientSettings &settings;
    client::world::WorldState world{};
    client::hud::HudFeed hud{};
    client::scenes::Starfield stars{120, 7};
    double accumulator = 0.0;
    double previous = 0.0;
    double hudPrevious = 0.0;
};

void receive(Game &game, double now)
{
    if (!game.world.configured() && game.session.welcome()) {
        game.world.configure(*game.session.welcome());
        game.hud.welcome(*game.session.welcome());
    }
    for (const auto &message : game.session.takeMessages()) {
        game.world.apply(message, now);
        game.hud.feed(message);
        playSounds(message, game.window);
    }
}

double stepOf(const Game &game)
{
    return 1.0 / static_cast<double>(game.world.rules().tickRate);
}

void sendInputs(Game &game, double now, Clock::time_point tick)
{
    game.accumulator += std::min(now - game.previous, 0.25);
    game.previous = now;
    while (game.accumulator >= stepOf(game) && game.world.configured()) {
        const std::uint8_t keys = readKeys(game.window, game.settings);

        game.world.predict(game.session.sendInput(keys, tick), keys);
        game.accumulator -= stepOf(game);
    }
}

void updateHud(Game &game, double now)
{
    const auto dt = static_cast<float>(std::min(now - game.hudPrevious, 0.25));
    const auto keys = readKeys(game.window, game.settings);

    game.hudPrevious = now;
    game.hud.update(dt, (keys & rtype::protocol::KEY_SHOOT) != 0);
}

client::render::Frame frameOf(const Game &game, double now)
{
    client::render::Frame frame{game.world,
                                game.stars,
                                now,
                                bannerOf(game.session, game.world),
                                static_cast<int>(game.session.rtt().count()),
                                game.settings.palette,
                                game.settings.highContrast};

    frame.hud = &game.hud.state();
    for (std::size_t side = 0; side < frame.cues.size(); ++side) {
        frame.cues[side] =
            game.hud.cue(static_cast<client::events::CueDirection>(side));
    }
    return frame;
}

void render(Game &game, double now)
{
    game.stars.advance(static_cast<float>(stepOf(game)));
    game.world.update(now);
    game.window.draw(frameOf(game, now));
}

bool quitRequested(const Game &game)
{
    const auto menuKey = client::settings::bindingOf(game.settings.bindings,
                                                     GameAction::OPEN_MENU)
                             .key;

    return game.window.isKeyDown(menuKey);
}

void runGame(ClientSession &session, client::render::GameWindow &window,
             const client::settings::ClientSettings &settings)
{
    const auto start = Clock::now();
    Game game{session, window, settings};

    while (window.isOpen()) {
        const auto tick = Clock::now();
        const double now = seconds(start, tick);

        window.pollEvents();
        if (quitRequested(game)) {
            window.close();
            break;
        }
        session.update(tick);
        receive(game, now);
        sendInputs(game, now, tick);
        updateHud(game, now);
        render(game, now);
    }
    session.disconnect(Clock::now());
}

} // namespace

namespace {

int play(const Options &options, const char *program)
{
    client::settings::SettingsStore store(settingsPath());

    store.load();
    ClientSession session(options.host, options.port, options.name);
    client::render::GameWindow window(store.settings(), "R-Type",
                                      findAssets(program));

    runGame(session, window, store.settings());
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    Options options;
    const std::span<char *> arguments(argv, static_cast<std::size_t>(argc));
    const auto rest = arguments.empty() ? arguments : arguments.subspan(1);
    const std::vector<std::string> args(rest.begin(), rest.end());

    if (!parseOptions(args, options)) {
        printUsage();
        return 1;
    }
    if (options.help) {
        printUsage();
        return 0;
    }
    return play(options, arguments.empty() ? "" : argv[0]);
}
