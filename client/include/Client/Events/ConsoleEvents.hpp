/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleEvents.hpp
*/

#ifndef CONSOLEEVENTS_HPP_
#define CONSOLEEVENTS_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace client::events {

/** @brief The player sent a command line. Run by the code that owns it. */
struct ConsoleCommandSubmitted {
    std::string line;
};

/** @brief Importance of a console line. */
enum class ConsoleLevel : std::uint8_t {
    INFO,
    WARNING,
    ERROR_LINE
};

/** @brief Something to print in the console. */
struct ConsoleOutput {
    std::string text;
    ConsoleLevel level = ConsoleLevel::INFO;
};

/** @brief The command names known so far, used for Tab completion. */
struct ConsoleCommandsListed {
    std::vector<std::string> names;
};

} // namespace client::events

#endif /* !CONSOLEEVENTS_HPP_ */
