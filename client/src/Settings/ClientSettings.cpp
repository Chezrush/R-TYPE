/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientSettings.cpp
*/

#include "Client/Settings/ClientSettings.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <optional>
#include <sstream>

namespace client::settings {

namespace {

constexpr int MAX_VOLUME = 100;

std::string trimmed(const std::string &text)
{
    const std::size_t first = text.find_first_not_of(" \t\r");

    if (first == std::string::npos)
        return "";
    return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
}

std::optional<int> parseInt(const std::string &text)
{
    int value = 0;
    const char *end = text.data() + text.size();
    const auto [last, error] = std::from_chars(text.data(), end, value);

    if (error != std::errc{} || last != end)
        return std::nullopt;
    return value;
}

void readVolume(const std::string &text, int &volume)
{
    const std::optional<int> value = parseInt(text);

    if (value.has_value() && *value >= 0 && *value <= MAX_VOLUME)
        volume = *value;
}

void readBool(const std::string &text, bool &flag)
{
    if (text == "true" || text == "false")
        flag = text == "true";
}

void readResolution(const std::string &text, ClientSettings &settings)
{
    for (const events::Resolution &resolution : RESOLUTIONS) {
        if (text == std::to_string(resolution.width) + "x" +
                        std::to_string(resolution.height))
            settings.resolution = resolution;
    }
}

void readUIScale(const std::string &text, ClientSettings &settings)
{
    const std::optional<int> value = parseInt(text);

    if (value.has_value() &&
        std::find(UI_SCALE_PERCENTS.begin(), UI_SCALE_PERCENTS.end(), *value) !=
            UI_SCALE_PERCENTS.end())
        settings.uiScalePercent = *value;
}

void readAudio(const std::string &key, const std::string &value,
               ClientSettings &settings)
{
    if (key == "master_volume")
        readVolume(value, settings.masterVolume);
    else if (key == "music_volume")
        readVolume(value, settings.musicVolume);
    else if (key == "sfx_volume")
        readVolume(value, settings.sfxVolume);
    else if (key == "muted")
        readBool(value, settings.muted);
}

void readVideo(const std::string &key, const std::string &value,
               ClientSettings &settings)
{
    if (key == "fullscreen")
        readBool(value, settings.fullscreen);
    else if (key == "resolution")
        readResolution(value, settings);
    else if (key == "vsync")
        readBool(value, settings.vsync);
    else if (key == "ui_scale")
        readUIScale(value, settings);
}

void readAccessibility(const std::string &key, const std::string &value,
                       ClientSettings &settings)
{
    if (key == "high_contrast")
        readBool(value, settings.highContrast);
    else if (key == "reduced_motion")
        readBool(value, settings.reducedMotion);
    else if (key != "palette")
        return;
    for (const ColorPalette palette : COLOR_PALETTES) {
        if (key == "palette" && value == paletteId(palette))
            settings.palette = palette;
    }
}

const char *boolText(bool flag) noexcept
{
    return flag ? "true" : "false";
}

} // namespace

std::string formatSettings(const ClientSettings &settings)
{
    std::ostringstream out;

    out << "master_volume=" << settings.masterVolume << '\n'
        << "music_volume=" << settings.musicVolume << '\n'
        << "sfx_volume=" << settings.sfxVolume << '\n'
        << "muted=" << boolText(settings.muted) << '\n'
        << "fullscreen=" << boolText(settings.fullscreen) << '\n'
        << "resolution=" << settings.resolution.width << 'x'
        << settings.resolution.height << '\n'
        << "vsync=" << boolText(settings.vsync) << '\n'
        << "ui_scale=" << settings.uiScalePercent << '\n';
    out << "palette=" << paletteId(settings.palette) << '\n'
        << "high_contrast=" << boolText(settings.highContrast) << '\n'
        << "reduced_motion=" << boolText(settings.reducedMotion) << '\n'
        << formatBindings(settings.bindings);
    return out.str();
}

ClientSettings parseSettings(const std::string &content)
{
    ClientSettings settings;
    std::istringstream lines(content);
    std::string line;

    while (std::getline(lines, line)) {
        const std::size_t equal = line.find('=');

        if (equal == std::string::npos)
            continue;
        const std::string key = trimmed(line.substr(0, equal));
        const std::string value = trimmed(line.substr(equal + 1));
        readAudio(key, value, settings);
        readVideo(key, value, settings);
        readAccessibility(key, value, settings);
        readBinding(key, value, settings.bindings);
    }
    return settings;
}

} // namespace client::settings
