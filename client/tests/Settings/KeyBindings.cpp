/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** KeyBindings.cpp
*/

#include "Client/Settings/KeyBindings.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

#include "Client/Settings/ClientSettings.hpp"

using namespace client::settings;

TEST_CASE("defaults use the arrow keys and the D-pad", "[client][bindings]")
{
    const KeyBindings bindings;

    REQUIRE(bindingOf(bindings, GameAction::MOVE_UP) ==
            ActionBinding{KEY_UP, GamepadButton::DPAD_UP});
    REQUIRE(bindingOf(bindings, GameAction::MOVE_DOWN) ==
            ActionBinding{KEY_DOWN, GamepadButton::DPAD_DOWN});
    REQUIRE(bindingOf(bindings, GameAction::MOVE_LEFT) ==
            ActionBinding{KEY_LEFT, GamepadButton::DPAD_LEFT});
    REQUIRE(bindingOf(bindings, GameAction::MOVE_RIGHT) ==
            ActionBinding{KEY_RIGHT, GamepadButton::DPAD_RIGHT});
    REQUIRE(findConflicts(bindings).empty());
    REQUIRE(conflictWarning(bindings).empty());
}

TEST_CASE("rebinding changes only that action", "[client][bindings]")
{
    KeyBindings bindings;

    setKey(bindings, GameAction::SHOOT, 0x1D);
    setButton(bindings, GameAction::SHOOT, GamepadButton::WEST);

    REQUIRE(bindingOf(bindings, GameAction::SHOOT) ==
            ActionBinding{0x1D, GamepadButton::WEST});
    REQUIRE(bindingOf(bindings, GameAction::MOVE_UP).key == KEY_UP);
}

TEST_CASE("sharing a key or a button is a conflict", "[client][bindings]")
{
    KeyBindings keys;
    KeyBindings buttons;

    setKey(keys, GameAction::SHOOT, KEY_UP);
    setButton(buttons, GameAction::OPEN_MENU, GamepadButton::SOUTH);

    const auto conflicts = findConflicts(keys);
    REQUIRE(conflicts.size() == 1);
    REQUIRE(conflicts[0].first == GameAction::MOVE_UP);
    REQUIRE(conflicts[0].second == GameAction::SHOOT);
    REQUIRE(conflictWarning(keys).find("Move up and Shoot") !=
            std::string::npos);
    REQUIRE(findConflicts(buttons).size() == 1);
}

TEST_CASE("keys and buttons have readable names", "[client][bindings]")
{
    REQUIRE(keyName(0x04) == "A");
    REQUIRE(keyName(0x1D) == "Z");
    REQUIRE(keyName(0x1E) == "1");
    REQUIRE(keyName(0x26) == "9");
    REQUIRE(keyName(0x27) == "0");
    REQUIRE(keyName(KEY_SPACE) == "Space");
    REQUIRE(keyName(KEY_UP) == "Up");
    REQUIRE(keyName(200) == "Key 200");
    REQUIRE(buttonName(GamepadButton::START) == "Start");
    REQUIRE(bindingText(KeyBindings{}, GameAction::MOVE_UP) == "Up / D-pad up");
}

TEST_CASE("bindings persist inside the settings text", "[client][bindings]")
{
    ClientSettings settings;
    setKey(settings.bindings, GameAction::SHOOT, 0x1D);
    setButton(settings.bindings, GameAction::OPEN_MENU, GamepadButton::SELECT);
    settings.bindings.autoFire = true;
    settings.bindings.beamMode = BeamChargeMode::TOGGLE;

    REQUIRE(parseSettings(formatSettings(settings)) == settings);
}

TEST_CASE("invalid binding lines keep the default", "[client][bindings]")
{
    const ClientSettings settings = parseSettings("key_shoot=0\n"
                                                  "key_move_up=9999\n"
                                                  "key_move_down=abc\n"
                                                  "pad_shoot=banana\n"
                                                  "auto_fire=maybe\n"
                                                  "beam_mode=sideways\n");

    REQUIRE(settings.bindings == KeyBindings{});
}
