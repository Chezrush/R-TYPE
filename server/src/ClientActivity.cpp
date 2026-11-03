/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ClientActivity
*/

#include "ClientActivity.hpp"

namespace rtype::server {

void ClientActivity::touch(std::uint32_t clientId, Clock::time_point now)
{
    const std::lock_guard<std::mutex> lock(_mutex);

    _lastSeen[clientId] = now;
}

bool ClientActivity::forget(std::uint32_t clientId)
{
    const std::lock_guard<std::mutex> lock(_mutex);

    return _lastSeen.erase(clientId) > 0;
}

std::vector<std::uint32_t> ClientActivity::takeExpired(Clock::time_point now,
                                                       Clock::duration timeout)
{
    const std::lock_guard<std::mutex> lock(_mutex);
    std::vector<std::uint32_t> expired;

    for (const auto &[clientId, lastSeen] : _lastSeen) {
        if (now - lastSeen > timeout) {
            expired.push_back(clientId);
        }
    }
    for (const std::uint32_t clientId : expired) {
        _lastSeen.erase(clientId);
    }
    return expired;
}

} /* namespace rtype::server */
