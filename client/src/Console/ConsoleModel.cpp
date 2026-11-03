/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleModel.cpp
*/

#include "Client/Console/ConsoleModel.hpp"

#include <algorithm>
#include <utility>

namespace client::console {

namespace {

std::string trimmed(const std::string &text)
{
    const std::size_t first = text.find_first_not_of(' ');

    if (first == std::string::npos)
        return "";
    return text.substr(first, text.find_last_not_of(' ') - first + 1);
}

std::string commonPrefix(const std::vector<std::string> &names)
{
    std::string prefix = names.front();

    for (const std::string &name : names) {
        const auto end = std::mismatch(prefix.begin(), prefix.end(),
                                       name.begin(), name.end());
        prefix.erase(end.first, prefix.end());
    }
    return prefix;
}

const char *prefixOf(events::ConsoleLevel level) noexcept
{
    switch (level) {
        case events::ConsoleLevel::WARNING:
            return "[warning] ";
        case events::ConsoleLevel::ERROR_LINE:
            return "[error] ";
        default:
            return "";
    }
}

} // namespace

void ConsoleModel::addOutput(const std::string &text,
                             events::ConsoleLevel level)
{
    _lines.push_back(prefixOf(level) + text);
    if (_lines.size() > MAX_LINES)
        _lines.pop_front();
}

std::optional<std::string> ConsoleModel::submit(const std::string &line)
{
    std::string command = trimmed(line);

    _browsing.reset();
    _draft.clear();
    if (command.empty())
        return std::nullopt;
    if (_history.empty() || _history.back() != command)
        _history.push_back(command);
    if (_history.size() > MAX_HISTORY)
        _history.erase(_history.begin());
    return command;
}

std::optional<std::string> ConsoleModel::previous(const std::string &current)
{
    if (_history.empty() || _browsing == 0)
        return std::nullopt;
    if (!_browsing.has_value()) {
        _draft = current;
        _browsing = _history.size();
    }
    _browsing = *_browsing - 1;
    return _history.at(*_browsing);
}

std::optional<std::string> ConsoleModel::next()
{
    if (!_browsing.has_value())
        return std::nullopt;
    _browsing = *_browsing + 1;
    if (*_browsing < _history.size())
        return _history.at(*_browsing);
    _browsing.reset();
    return _draft;
}

void ConsoleModel::setCommands(std::vector<std::string> names)
{
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
    _commands = std::move(names);
}

Completion ConsoleModel::complete(const std::string &text) const
{
    std::vector<std::string> matches;

    if (text.find(' ') != std::string::npos)
        return {text, {}};
    for (const std::string &name : _commands) {
        if (name.compare(0, text.size(), text) == 0)
            matches.push_back(name);
    }
    if (matches.empty())
        return {text, {}};
    if (matches.size() == 1)
        return {matches.front() + " ", {}};
    return {commonPrefix(matches), matches};
}

const std::deque<std::string> &ConsoleModel::lines() const noexcept
{
    return _lines;
}

const std::vector<std::string> &ConsoleModel::history() const noexcept
{
    return _history;
}

} // namespace client::console
