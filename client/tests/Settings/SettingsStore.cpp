/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SettingsStore.cpp
*/

#include "Client/Settings/SettingsStore.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>

#include "TempFile.hpp"

using client::settings::ClientSettings;
using client::settings::defaultSettingsPath;
using client::settings::SettingsStore;

TEST_CASE("settings persist across restarts", "[client][settings][file]")
{
    const TempDirectory directory;
    const auto path = directory.path() / "deep" / "er" / "settings.cfg";
    {
        SettingsStore store(path);
        store.settings().masterVolume = 35;
        store.settings().uiScalePercent = 125;
        REQUIRE(store.save());
    }
    SettingsStore restarted(path);

    restarted.load();

    REQUIRE(restarted.settings().masterVolume == 35);
    REQUIRE(restarted.settings().uiScalePercent == 125);
    REQUIRE_FALSE(std::filesystem::exists(path.string() + ".tmp"));
}

TEST_CASE("a missing file gives the defaults", "[client][settings][file]")
{
    const TempDirectory directory;
    SettingsStore store(directory.path() / "nothing.cfg");

    store.settings().masterVolume = 1;
    store.load();

    REQUIRE(store.settings() == ClientSettings{});
}

TEST_CASE("a corrupted file gives usable settings", "[client][settings][file]")
{
    const TempDirectory directory;
    const auto path = directory.path() / "settings.cfg";
    {
        std::ofstream file(path, std::ios::binary);
        file << std::string(100, '\xFF') << "\nmaster_volume=20\n"
             << std::string(1, '\0') << "muted=????\n";
    }
    SettingsStore store(path);

    store.load();

    REQUIRE(store.settings().masterVolume == 20);
    REQUIRE_FALSE(store.settings().muted);
}

TEST_CASE("a directory instead of a file changes nothing dangerous",
          "[client][settings][file]")
{
    const TempDirectory directory;
    SettingsStore store(directory.path());

    store.load();

    REQUIRE(store.settings() == ClientSettings{});
    REQUIRE_FALSE(store.save());
}

TEST_CASE("the default file is named settings.cfg", "[client][settings][file]")
{
    REQUIRE(defaultSettingsPath().filename() == "settings.cfg");
}
