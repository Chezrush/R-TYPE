/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** KeyBindings.hpp
*/

#ifndef KEYBINDINGS_HPP_
#define KEYBINDINGS_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace client::settings {

/** @brief What the player can do during the game. */
enum class GameAction : std::uint8_t {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    SHOOT,
    OPEN_MENU
};

inline constexpr std::array<GameAction, 6> GAME_ACTIONS{
    GameAction::MOVE_UP,    GameAction::MOVE_DOWN, GameAction::MOVE_LEFT,
    GameAction::MOVE_RIGHT, GameAction::SHOOT,     GameAction::OPEN_MENU};

/** @brief Standard gamepad buttons; the window code maps its device on them. */
enum class GamepadButton : std::uint8_t {
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    SOUTH,
    EAST,
    WEST,
    NORTH,
    LEFT_SHOULDER,
    RIGHT_SHOULDER,
    START,
    SELECT
};

/**
 * @brief Physical key, as a USB HID usage code.
 *
 * It names a position on the keyboard, not a letter, so the same key works on
 * QWERTY and AZERTY. It depends on no window library: the window code converts
 * its own scancodes to it.
 */
using ScanCode = std::uint16_t;

inline constexpr ScanCode KEY_ESCAPE = 0x29;
inline constexpr ScanCode KEY_SPACE = 0x2C;
inline constexpr ScanCode KEY_RIGHT = 0x4F;
inline constexpr ScanCode KEY_LEFT = 0x50;
inline constexpr ScanCode KEY_DOWN = 0x51;
inline constexpr ScanCode KEY_UP = 0x52;

/** @brief The key and the gamepad button of one action. */
struct ActionBinding {
    ScanCode key = 0;
    GamepadButton button = GamepadButton::SOUTH;

    friend bool operator==(const ActionBinding &,
                           const ActionBinding &) = default;
};

/** @brief Whether the beam charges while shoot is held, or between presses. */
enum class BeamChargeMode : std::uint8_t {
    HOLD,
    TOGGLE
};

/** @brief The controls chosen by the player. */
struct KeyBindings {
    std::array<ActionBinding, GAME_ACTIONS.size()> actions{{
        {KEY_UP, GamepadButton::DPAD_UP},
        {KEY_DOWN, GamepadButton::DPAD_DOWN},
        {KEY_LEFT, GamepadButton::DPAD_LEFT},
        {KEY_RIGHT, GamepadButton::DPAD_RIGHT},
        {KEY_SPACE, GamepadButton::SOUTH},
        {KEY_ESCAPE, GamepadButton::START},
    }};
    bool autoFire = false;
    BeamChargeMode beamMode = BeamChargeMode::HOLD;

    friend bool operator==(const KeyBindings &, const KeyBindings &) = default;
};

[[nodiscard]] const ActionBinding &bindingOf(const KeyBindings &bindings,
                                             GameAction action);
void setKey(KeyBindings &bindings, GameAction action, ScanCode key);
void setButton(KeyBindings &bindings, GameAction action, GamepadButton button);

/** @brief Pairs of actions that share a key or a gamepad button. */
[[nodiscard]] std::vector<std::pair<GameAction, GameAction>>
findConflicts(const KeyBindings &bindings);

/** @brief "Move up", "Shoot"... */
[[nodiscard]] std::string actionName(GameAction action);

/** @brief "Up", "Space", "A", "Key 200"... named as on a QWERTY keyboard. */
[[nodiscard]] std::string keyName(ScanCode key);

/** @brief "D-pad up", "South button"... */
[[nodiscard]] std::string buttonName(GamepadButton button);

/** @brief "Up / D-pad up" */
[[nodiscard]] std::string bindingText(const KeyBindings &bindings,
                                      GameAction action);

/** @brief "Warning: ..." for the first conflict, or an empty text. */
[[nodiscard]] std::string conflictWarning(const KeyBindings &bindings);

/** @brief The bindings as key=value lines. */
[[nodiscard]] std::string formatBindings(const KeyBindings &bindings);

/** @brief Applies one key=value line if it is about valid bindings. */
void readBinding(const std::string &key, const std::string &value,
                 KeyBindings &bindings);

} // namespace client::settings

#endif /* !KEYBINDINGS_HPP_ */
