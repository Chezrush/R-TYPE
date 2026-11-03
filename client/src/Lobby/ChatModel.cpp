/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ChatModel.cpp
*/

#include "Client/Lobby/ChatModel.hpp"

#include <charconv>

#include "Client/Events/GameEvents.hpp"

namespace client::lobby {

namespace {

constexpr int SECONDS_PER_MINUTE = 60;

std::string twoDigits(long value)
{
    return (value < 10 ? "0" : "") + std::to_string(value);
}

std::optional<std::uint8_t> playerNumber(const std::string &text)
{
    int number = 0;
    const char *end = text.data() + text.size();
    const auto [last, error] = std::from_chars(text.data(), end, number);

    if (error != std::errc{} || last != end || number < 1 ||
        number > static_cast<int>(events::LOBBY_SLOTS))
        return std::nullopt;
    return static_cast<std::uint8_t>(number - 1);
}

ChatCommand playerCommand(ChatCommand::Kind kind, const std::string &argument)
{
    const auto player = playerNumber(argument);

    if (!player.has_value())
        return {ChatCommand::Kind::INVALID, "Use /mute or /unmute with 1 to 4.",
                0};
    return {kind, "", *player};
}

} // namespace

ChatCommand ChatModel::parse(const std::string &line)
{
    const std::size_t first = line.find_first_not_of(' ');

    if (first == std::string::npos)
        return {};
    const std::string text = line.substr(first, MAX_CHAT_LENGTH);

    if (text.rfind("/mute ", 0) == 0)
        return playerCommand(ChatCommand::Kind::MUTE, text.substr(6));
    if (text.rfind("/unmute ", 0) == 0)
        return playerCommand(ChatCommand::Kind::UNMUTE, text.substr(8));
    return {ChatCommand::Kind::SEND, text, 0};
}

std::string ChatModel::timestamp(double seconds)
{
    const long total = seconds > 0.0 ? static_cast<long>(seconds) : 0;

    return "[" + twoDigits(total / SECONDS_PER_MINUTE) + ":" +
           twoDigits(total % SECONDS_PER_MINUTE) + "]";
}

void ChatModel::add(const events::ChatMessageReceived &message, double seconds)
{
    const bool fromServer = message.player == events::NO_PLAYER;
    const std::string author = fromServer ? "*" : message.nickname + ":";

    _lines.push_back({message.player,
                      timestamp(seconds) + " " + author + " " + message.text});
    if (_lines.size() > MAX_LINES)
        _lines.pop_front();
}

void ChatModel::addNotice(const std::string &text, double seconds)
{
    add(events::ChatMessageReceived{events::NO_PLAYER, "", text}, seconds);
}

void ChatModel::setMuted(std::uint8_t player, bool muted) noexcept
{
    if (player < _muted.size())
        _muted[player] = muted;
}

bool ChatModel::isMuted(std::uint8_t player) const noexcept
{
    return player < _muted.size() && _muted[player];
}

std::vector<std::string> ChatModel::lines() const
{
    std::vector<std::string> visible;

    for (const Line &line : _lines) {
        if (!isMuted(line.player))
            visible.push_back(line.text);
    }
    return visible;
}

} // namespace client::lobby
