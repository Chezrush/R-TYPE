/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientSettings.cpp
*/

#include "Client/Settings/ClientSettings.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

using client::events::Resolution;
using client::settings::ClientSettings;
using client::settings::formatSettings;
using client::settings::parseSettings;

namespace {

ClientSettings customSettings()
{
    ClientSettings settings;

    settings.masterVolume = 55;
    settings.musicVolume = 0;
    settings.sfxVolume = 100;
    settings.muted = true;
    settings.fullscreen = true;
    settings.resolution = Resolution{1920, 1080};
    settings.vsync = false;
    settings.uiScalePercent = 150;
    return settings;
}

} // namespace

TEST_CASE("settings survive being written and read", "[client][settings]")
{
    REQUIRE(parseSettings(formatSettings(customSettings())) ==
            customSettings());
    REQUIRE(parseSettings(formatSettings({})) == ClientSettings{});
}

TEST_CASE("empty content gives the defaults", "[client][settings]")
{
    REQUIRE(parseSettings("") == ClientSettings{});
}

TEST_CASE("spaces and Windows line ends are accepted", "[client][settings]")
{
    const ClientSettings settings =
        parseSettings("  master_volume = 40 \r\nmuted=true\r\n");

    REQUIRE(settings.masterVolume == 40);
    REQUIRE(settings.muted);
}

TEST_CASE("invalid lines are ignored, the others still count",
          "[client][settings]")
{
    const ClientSettings settings = parseSettings("garbage\n"
                                                  "=\n"
                                                  "unknown=3\n"
                                                  "master_volume=abc\n"
                                                  "music_volume=101\n"
                                                  "sfx_volume=-1\n"
                                                  "muted=maybe\n"
                                                  "resolution=123x456\n"
                                                  "ui_scale=130\n"
                                                  "vsync=false\n");
    ClientSettings expected;
    expected.vsync = false;

    REQUIRE(settings == expected);
}

TEST_CASE("a number too big for an int is ignored", "[client][settings]")
{
    REQUIRE(parseSettings("master_volume=99999999999999999999") ==
            ClientSettings{});
}

TEST_CASE("binary garbage never breaks parsing", "[client][settings]")
{
    std::string garbage;

    for (int value = 0; value < 1024; ++value)
        garbage.push_back(static_cast<char>(value % 256));

    REQUIRE(parseSettings(garbage) == ClientSettings{});
}
