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

#include "Client/Flow/ClientApp.hpp"
#include "Client/Flow/IGameDevice.hpp"
#include "Client/Render/GameWindow.hpp"
#include "Client/Settings/SettingsStore.hpp"

namespace {

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
                 "  The options fill the connection form of the menu. "
                 "Defaults: 127.0.0.1,\n  4242, Player. Arrow keys move, space "
                 "shoots, Escape opens the menu.\n  Keys and display can be "
                 "changed in the settings screen.\n";
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

class WindowDevice : public client::flow::IGameDevice {
public:
    explicit WindowDevice(client::render::GameWindow &window)
        : _window(window)
    {}

    [[nodiscard]] bool isKeyDown(client::settings::ScanCode key) const override
    {
        return client::render::GameWindow::isKeyDown(key);
    }

    void playShot() override
    {
        _window.playShot();
    }

    void playExplosion() override
    {
        _window.playExplosion();
    }

    [[nodiscard]] engine::core::Vec2
    toScreen(engine::core::Vec2 world) const override
    {
        return _window.toScreen(world);
    }

    [[nodiscard]] engine::core::Vec2 size() const override
    {
        return _window.size();
    }

private:
    client::render::GameWindow &_window;
};

float secondsBetween(Clock::time_point start, Clock::time_point now)
{
    constexpr double MAX_FRAME = 0.25;

    return static_cast<float>(std::min(
        std::chrono::duration<double>(now - start).count(), MAX_FRAME));
}

void runLoop(client::render::GameWindow &window, client::flow::ClientApp &app)
{
    auto previous = Clock::now();

    while (window.isOpen() && !app.quitRequested()) {
        const auto now = Clock::now();

        window.pollEvents();
        app.frame(secondsBetween(previous, now));
        previous = now;
        if (const auto frame = app.gameFrame()) {
            window.draw(*frame);
        } else {
            window.drawUi();
        }
    }
    app.shutdown();
}

int play(const Options &options, const char *program)
{
    client::settings::SettingsStore store(settingsPath());

    store.load();
    client::render::GameWindow window(store.settings(), "R-Type",
                                      findAssets(program));
    WindowDevice device(window);
    client::flow::ClientApp app(
        {device,
         window.registry(),
         window.bus(),
         window.interaction(),
         window.layout(),
         window.theme(),
         store,
         window.fontId(),
         {options.host, std::to_string(options.port), options.name}});

    runLoop(window, app);
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
