/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SettingsStore.cpp
*/

#include "Client/Settings/SettingsStore.hpp"

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace client::settings {

namespace {

namespace fs = std::filesystem;

constexpr std::size_t MAX_FILE_SIZE = 65536;

std::string environment(const char *name)
{
#if defined(_WIN32)
    char *value = nullptr;
    std::size_t size = 0;
    std::string text;

    if (_dupenv_s(&value, &size, name) == 0 && value != nullptr)
        text = value;
    std::free(value);
    return text;
#else
    const char *value = std::getenv(name);

    return value != nullptr ? value : "";
#endif
}

fs::path dataDirectory()
{
#if defined(_WIN32)
    return fs::path(environment("APPDATA")) / "R-Type";
#elif defined(__APPLE__)
    return fs::path(environment("HOME")) / "Library/Application Support/R-Type";
#else
    const std::string dataHome = environment("XDG_DATA_HOME");

    if (!dataHome.empty())
        return fs::path(dataHome) / "r-type";
    return fs::path(environment("HOME")) / ".local/share/r-type";
#endif
}

std::string readLimited(const fs::path &path)
{
    std::ifstream file(path, std::ios::binary);
    std::vector<char> buffer(MAX_FILE_SIZE);

    if (!file)
        return "";
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    return {buffer.data(), static_cast<std::size_t>(file.gcount())};
}

} // namespace

fs::path defaultSettingsPath()
{
    return dataDirectory() / "settings.cfg";
}

SettingsStore::SettingsStore(fs::path path)
    : _path(std::move(path))
{}

void SettingsStore::load()
{
    _settings = parseSettings(readLimited(_path));
}

bool SettingsStore::save() const
{
    std::error_code error;
    fs::path temporary = _path;

    temporary += ".tmp";
    if (_path.has_parent_path())
        fs::create_directories(_path.parent_path(), error);
    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);

        file << formatSettings(_settings);
        if (!file.flush())
            return false;
    }
    fs::rename(temporary, _path, error);
    return !error;
}

ClientSettings &SettingsStore::settings() noexcept
{
    return _settings;
}

const ClientSettings &SettingsStore::settings() const noexcept
{
    return _settings;
}

} // namespace client::settings
