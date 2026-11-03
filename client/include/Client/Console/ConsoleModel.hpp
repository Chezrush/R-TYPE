/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleModel.hpp
*/

#ifndef CONSOLEMODEL_HPP_
#define CONSOLEMODEL_HPP_

#include <cstddef>
#include <deque>
#include <optional>
#include <string>
#include <vector>

#include "Client/Events/ConsoleEvents.hpp"

namespace client::console {

/** @brief Result of Tab completion. */
struct Completion {
    std::string text;
    std::vector<std::string> candidates;

    friend bool operator==(const Completion &, const Completion &) = default;
};

/**
 * @brief Log, history and completion of the console, with no UI.
 *
 * History is browsed like a shell: previous() goes back from the line being
 * typed, which next() gives back at the end. Only the command name (the first
 * word) is completed.
 */
class ConsoleModel {
public:
    static constexpr std::size_t MAX_LINES = 200;
    static constexpr std::size_t MAX_HISTORY = 50;

    /** @brief Adds a line to the log, "[warning] " / "[error] " prefixed. */
    void addOutput(const std::string &text, events::ConsoleLevel level);

    /**
     * @brief Records a submitted line.
     * @return The line without its outer spaces, or nothing if it is empty.
     */
    std::optional<std::string> submit(const std::string &line);

    /** @brief The older history entry, or nothing at the oldest one. */
    std::optional<std::string> previous(const std::string &current);

    /** @brief The newer entry, then the line that was being typed. */
    std::optional<std::string> next();

    /** @brief Replaces the known command names. */
    void setCommands(std::vector<std::string> names);

    [[nodiscard]] Completion complete(const std::string &text) const;

    [[nodiscard]] const std::deque<std::string> &lines() const noexcept;
    [[nodiscard]] const std::vector<std::string> &history() const noexcept;

private:
    std::deque<std::string> _lines;
    std::vector<std::string> _history;
    std::vector<std::string> _commands;
    std::optional<std::size_t> _browsing;
    std::string _draft;
};

} // namespace client::console

#endif /* !CONSOLEMODEL_HPP_ */
