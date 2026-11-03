/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** KeyBindings.cpp
*/

#include "Client/Settings/KeyBindings.hpp"

#include <charconv>
#include <optional>
#include <sstream>

namespace client::settings {

namespace {

struct NamedAction {
    const char *id;
    const char *name;
};

struct NamedButton {
    const char *id;
    const char *name;
};

struct NamedKey {
    ScanCode key;
    const char *name;
};

constexpr std::array<NamedAction, GAME_ACTIONS.size()> ACTIONS{{
    {"move_up", "Move up"},
    {"move_down", "Move down"},
    {"move_left", "Move left"},
    {"move_right", "Move right"},
    {"shoot", "Shoot"},
    {"open_menu", "Open menu"},
}};

constexpr std::array<NamedButton, 12> BUTTONS{{
    {"dpad_up", "D-pad up"},
    {"dpad_down", "D-pad down"},
    {"dpad_left", "D-pad left"},
    {"dpad_right", "D-pad right"},
    {"south", "South button"},
    {"east", "East button"},
    {"west", "West button"},
    {"north", "North button"},
    {"left_shoulder", "Left shoulder"},
    {"right_shoulder", "Right shoulder"},
    {"start", "Start"},
    {"select", "Select"},
}};

constexpr std::array<NamedKey, 16> KEYS{{
    {0x28, "Enter"},
    {KEY_ESCAPE, "Escape"},
    {0x2A, "Backspace"},
    {0x2B, "Tab"},
    {KEY_SPACE, "Space"},
    {KEY_GRAVE, "Grave"},
    {KEY_RIGHT, "Right"},
    {KEY_LEFT, "Left"},
    {KEY_DOWN, "Down"},
    {KEY_UP, "Up"},
    {0xE0, "Left Ctrl"},
    {0xE1, "Left Shift"},
    {0xE2, "Left Alt"},
    {0xE4, "Right Ctrl"},
    {0xE5, "Right Shift"},
    {0xE6, "Right Alt"},
}};

constexpr ScanCode FIRST_LETTER = 0x04;
constexpr ScanCode LAST_LETTER = 0x1D;
constexpr ScanCode FIRST_DIGIT = 0x1E;
constexpr ScanCode LAST_DIGIT = 0x26;
constexpr ScanCode ZERO_DIGIT = 0x27;
constexpr int MAX_SCAN_CODE = 0xFF;

std::size_t indexOf(GameAction action) noexcept
{
    return static_cast<std::size_t>(action);
}

std::optional<ScanCode> parseKey(const std::string &text)
{
    int value = 0;
    const char *end = text.data() + text.size();
    const auto [last, error] = std::from_chars(text.data(), end, value);

    if (error != std::errc{} || last != end || value < 1 ||
        value > MAX_SCAN_CODE)
        return std::nullopt;
    return static_cast<ScanCode>(value);
}

std::optional<GamepadButton> parseButton(const std::string &text)
{
    for (std::size_t i = 0; i < BUTTONS.size(); ++i) {
        if (text == BUTTONS.at(i).id)
            return static_cast<GamepadButton>(i);
    }
    return std::nullopt;
}

bool sameControl(const ActionBinding &first, const ActionBinding &second)
{
    return first.key == second.key || first.button == second.button;
}

void readOptions(const std::string &key, const std::string &value,
                 KeyBindings &bindings)
{
    if (key == "auto_fire" && (value == "true" || value == "false"))
        bindings.autoFire = value == "true";
    if (key == "beam_mode" && value == "hold")
        bindings.beamMode = BeamChargeMode::HOLD;
    if (key == "beam_mode" && value == "toggle")
        bindings.beamMode = BeamChargeMode::TOGGLE;
}

} // namespace

const ActionBinding &bindingOf(const KeyBindings &bindings, GameAction action)
{
    return bindings.actions.at(indexOf(action));
}

void setKey(KeyBindings &bindings, GameAction action, ScanCode key)
{
    bindings.actions.at(indexOf(action)).key = key;
}

void setButton(KeyBindings &bindings, GameAction action, GamepadButton button)
{
    bindings.actions.at(indexOf(action)).button = button;
}

std::vector<std::pair<GameAction, GameAction>>
findConflicts(const KeyBindings &bindings)
{
    std::vector<std::pair<GameAction, GameAction>> conflicts;

    for (std::size_t i = 0; i < GAME_ACTIONS.size(); ++i) {
        for (std::size_t j = i + 1; j < GAME_ACTIONS.size(); ++j) {
            if (sameControl(bindings.actions.at(i), bindings.actions.at(j)))
                conflicts.emplace_back(GAME_ACTIONS.at(i), GAME_ACTIONS.at(j));
        }
    }
    return conflicts;
}

std::string actionName(GameAction action)
{
    return ACTIONS.at(indexOf(action)).name;
}

std::string keyName(ScanCode key)
{
    if (key >= FIRST_LETTER && key <= LAST_LETTER)
        return std::string(1, static_cast<char>('A' + (key - FIRST_LETTER)));
    if (key >= FIRST_DIGIT && key <= LAST_DIGIT)
        return std::string(1, static_cast<char>('1' + (key - FIRST_DIGIT)));
    if (key == ZERO_DIGIT)
        return "0";
    for (const NamedKey &named : KEYS) {
        if (named.key == key)
            return named.name;
    }
    return "Key " + std::to_string(key);
}

std::string buttonName(GamepadButton button)
{
    return BUTTONS.at(static_cast<std::size_t>(button)).name;
}

std::string bindingText(const KeyBindings &bindings, GameAction action)
{
    const ActionBinding &binding = bindingOf(bindings, action);

    return keyName(binding.key) + " / " + buttonName(binding.button);
}

std::string conflictWarning(const KeyBindings &bindings)
{
    const auto conflicts = findConflicts(bindings);

    if (conflicts.empty())
        return "";
    return "Warning: " + actionName(conflicts.front().first) + " and " +
           actionName(conflicts.front().second) + " use the same control.";
}

std::string formatBindings(const KeyBindings &bindings)
{
    std::ostringstream out;

    for (std::size_t i = 0; i < ACTIONS.size(); ++i) {
        const ActionBinding &binding = bindings.actions.at(i);

        out << "key_" << ACTIONS.at(i).id << '=' << binding.key << '\n'
            << "pad_" << ACTIONS.at(i).id << '='
            << BUTTONS.at(static_cast<std::size_t>(binding.button)).id << '\n';
    }
    out << "auto_fire=" << (bindings.autoFire ? "true" : "false") << '\n'
        << "beam_mode="
        << (bindings.beamMode == BeamChargeMode::TOGGLE ? "toggle" : "hold")
        << '\n';
    return out.str();
}

void readBinding(const std::string &key, const std::string &value,
                 KeyBindings &bindings)
{
    readOptions(key, value, bindings);
    for (std::size_t i = 0; i < ACTIONS.size(); ++i) {
        const std::string id = ACTIONS.at(i).id;
        const auto scanCode = parseKey(value);
        const auto button = parseButton(value);

        if (key == "key_" + id && scanCode.has_value())
            bindings.actions.at(i).key = *scanCode;
        if (key == "pad_" + id && button.has_value())
            bindings.actions.at(i).button = *button;
    }
}

} // namespace client::settings
