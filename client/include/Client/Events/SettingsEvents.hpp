/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SettingsEvents.hpp
*/

#ifndef SETTINGSEVENTS_HPP_
#define SETTINGSEVENTS_HPP_

#include <cstdint>

#include "Client/Settings/KeyBindings.hpp"

namespace client::events {

/** @brief Volume channels. */
enum class VolumeBus : std::uint8_t {
    MASTER,
    MUSIC,
    SFX
};

/** @brief Size of the window, in pixels. */
struct Resolution {
    unsigned int width = 1280;
    unsigned int height = 720;

    friend bool operator==(const Resolution &, const Resolution &) = default;
};

/** @brief A volume changed, from 0 to 100. Applied by the audio code. */
struct VolumeChanged {
    VolumeBus bus = VolumeBus::MASTER;
    int value = 100;
};

/** @brief The sound was muted or unmuted. Applied by the audio code. */
struct MuteChanged {
    bool muted = false;
};

/** @brief A video setting changed. Applied by the window code. */
struct VideoSettingsChanged {
    bool fullscreen = false;
    Resolution resolution;
    bool vsync = true;
};

/** @brief The player changed "reduced motion", for the rest of the game. */
struct ReducedMotionChanged {
    bool reduced = false;
};

/** @brief The controls changed: the input code reloads them. */
struct BindingsChanged {
    settings::KeyBindings bindings;
};

/** @brief A key went down. Published by the window code, for key capture. */
struct RawKeyPressed {
    settings::ScanCode key = 0;
};

/** @brief A gamepad button went down. Published by the window code. */
struct RawGamepadButtonPressed {
    settings::GamepadButton button = settings::GamepadButton::SOUTH;
};

} // namespace client::events

#endif /* !SETTINGSEVENTS_HPP_ */
