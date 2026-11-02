/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** TempFile.hpp
*/

#ifndef TEMPFILE_HPP_
#define TEMPFILE_HPP_

#include <filesystem>
#include <random>
#include <string>
#include <system_error>

/** @brief A private temporary directory, removed with this object. */
class TempDirectory {
public:
    TempDirectory()
        : _path(std::filesystem::temp_directory_path() /
                ("rtype-settings-" + std::to_string(std::random_device{}())))
    {
        std::filesystem::create_directories(_path);
    }

    ~TempDirectory()
    {
        std::error_code ignored;

        std::filesystem::remove_all(_path, ignored);
    }

    TempDirectory(const TempDirectory &) = delete;
    TempDirectory &operator=(const TempDirectory &) = delete;
    TempDirectory(TempDirectory &&) = delete;
    TempDirectory &operator=(TempDirectory &&) = delete;

    [[nodiscard]] const std::filesystem::path &path() const noexcept
    {
        return _path;
    }

private:
    std::filesystem::path _path;
};

#endif /* !TEMPFILE_HPP_ */
