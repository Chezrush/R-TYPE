/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SettingsStore.hpp
*/

#ifndef SETTINGSSTORE_HPP_
#define SETTINGSSTORE_HPP_

#include <filesystem>

#include "Client/Settings/ClientSettings.hpp"

namespace client::settings {

/** @brief File of the settings in the user data directory of this system. */
[[nodiscard]] std::filesystem::path defaultSettingsPath();

/**
 * @brief The settings of the player, and the file that keeps them.
 *
 * A missing or broken file gives usable settings: nothing here throws or stops
 * the game because of the disk.
 */
class SettingsStore {
public:
    explicit SettingsStore(std::filesystem::path path);

    /** @brief Replaces the settings with the content of the file. */
    void load();

    /**
     * @brief Writes the file, creating its directory if needed.
     *
     * The old file stays complete until the new one is written.
     * @return false if the file could not be written.
     */
    bool save() const;

    [[nodiscard]] ClientSettings &settings() noexcept;
    [[nodiscard]] const ClientSettings &settings() const noexcept;

private:
    std::filesystem::path _path;
    ClientSettings _settings;
};

} // namespace client::settings

#endif /* !SETTINGSSTORE_HPP_ */
