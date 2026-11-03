/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectForm.cpp
*/

#include "Client/Scenes/ConnectForm.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace client::scenes {

namespace {

constexpr std::size_t MAX_HOST_LENGTH = 253;
constexpr std::size_t MAX_LABEL_LENGTH = 63;
constexpr std::size_t IPV4_PARTS = 4;
constexpr std::size_t MAX_OCTET_DIGITS = 3;
constexpr int MAX_OCTET = 255;
constexpr std::size_t MAX_PORT_DIGITS = 5;
constexpr int MAX_PORT = 65535;

bool isDigit(char character) noexcept
{
    return character >= '0' && character <= '9';
}

bool isLetter(char character) noexcept
{
    return (character >= 'a' && character <= 'z') ||
           (character >= 'A' && character <= 'Z');
}

bool isNumber(const std::string &text) noexcept
{
    return !text.empty() && std::all_of(text.begin(), text.end(), isDigit);
}

std::vector<std::string> splitOnDots(const std::string &text)
{
    std::vector<std::string> parts(1);

    for (const char character : text) {
        if (character == '.')
            parts.emplace_back();
        else
            parts.back().push_back(character);
    }
    return parts;
}

bool isValidOctet(const std::string &part)
{
    return isNumber(part) && part.size() <= MAX_OCTET_DIGITS &&
           std::stoi(part) <= MAX_OCTET;
}

bool isValidIpv4(const std::vector<std::string> &parts)
{
    return parts.size() == IPV4_PARTS &&
           std::all_of(parts.begin(), parts.end(), isValidOctet);
}

bool isValidLabel(const std::string &label)
{
    const auto allowed = [](char character) {
        return isDigit(character) || isLetter(character) || character == '-';
    };

    return !label.empty() && label.size() <= MAX_LABEL_LENGTH &&
           label.front() != '-' && label.back() != '-' &&
           std::all_of(label.begin(), label.end(), allowed);
}

} // namespace

bool isValidHost(const std::string &host)
{
    if (host.empty() || host.size() > MAX_HOST_LENGTH)
        return false;
    const std::vector<std::string> parts = splitOnDots(host);

    // A name ending with a number would be read as a broken address.
    if (isNumber(parts.back()))
        return isValidIpv4(parts);
    return std::all_of(parts.begin(), parts.end(), isValidLabel);
}

std::optional<std::uint16_t> parsePort(const std::string &port)
{
    if (!isNumber(port) || port.size() > MAX_PORT_DIGITS)
        return std::nullopt;
    const int value = std::stoi(port);

    if (value < 1 || value > MAX_PORT)
        return std::nullopt;
    return static_cast<std::uint16_t>(value);
}

bool isValidNickname(const std::string &nickname)
{
    const auto printable = [](char character) {
        return character >= 0x20 && character <= 0x7E;
    };

    return !nickname.empty() && nickname.size() <= MAX_NICKNAME_BYTES &&
           std::all_of(nickname.begin(), nickname.end(), printable);
}

std::optional<std::string> validationError(const ConnectForm &form)
{
    if (!isValidHost(form.host))
        return "Invalid address: use an IPv4 address or a host name.";
    if (!parsePort(form.port).has_value())
        return "Invalid port: use a number from 1 to 65535.";
    if (!isValidNickname(form.nickname))
        return "Invalid nickname: use 1 to 16 plain characters (no accents).";
    return std::nullopt;
}

std::string connectingMessage(const std::string &host, std::uint16_t port)
{
    return "Connecting to " + host + ":" + std::to_string(port) + "...";
}

std::string failureMessage(events::ConnectionFailure reason)
{
    using events::ConnectionFailure;

    switch (reason) {
        case ConnectionFailure::SERVER_FULL:
            return "The server is full. Try again later.";
        case ConnectionFailure::INVALID_NAME:
            return "The server refused this nickname. Choose another one.";
        case ConnectionFailure::VERSION:
            return "This server runs another version of the game.";
        case ConnectionFailure::RATE_LIMITED:
            return "Too many attempts. Wait a moment and try again.";
        case ConnectionFailure::SHUTTING_DOWN:
            return "The server is shutting down.";
        case ConnectionFailure::TIMEOUT:
            return "The server did not answer. Check the address and the port.";
        case ConnectionFailure::INVALID_ADDRESS:
            return "This address could not be found.";
    }
    return "The server refused the connection.";
}

} // namespace client::scenes
