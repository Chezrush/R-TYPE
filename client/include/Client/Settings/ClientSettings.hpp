/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientSettings.hpp
*/

#ifndef CLIENTSETTINGS_HPP_
#define CLIENTSETTINGS_HPP_

#include <array>
#include <string>

#include "Client/Events/SettingsEvents.hpp"

namespace client::settings {

/** @brief Resolutions offered by the settings screen. */
inline constexpr std::array<events::Resolution, 4> RESOLUTIONS{{
    {1280, 720},
    {1600, 900},
    {1920, 1080},
    {2560, 1440},
}};

/** @brief UI scales offered by the settings screen, in percent. */
inline constexpr std::array<int, 3> UI_SCALE_PERCENTS{100, 125, 150};

/** @brief What the player chose in the settings screen. */
struct ClientSettings {
    int masterVolume = 100;
    int musicVolume = 80;
    int sfxVolume = 80;
    bool muted = false;
    bool fullscreen = false;
    events::Resolution resolution;
    bool vsync = true;
    int uiScalePercent = 100;

    friend bool operator==(const ClientSettings &,
                           const ClientSettings &) = default;
};

/** @brief The settings as key=value lines. */
[[nodiscard]] std::string formatSettings(const ClientSettings &settings);

/**
 * @brief Reads key=value lines, starting from the default settings.
 *
 * Anything else is ignored line by line: unknown keys, values out of range,
 * garbage. So any content gives valid settings.
 */
[[nodiscard]] ClientSettings parseSettings(const std::string &content);

} // namespace client::settings

#endif /* !CLIENTSETTINGS_HPP_ */
