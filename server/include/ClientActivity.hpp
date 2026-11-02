/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ClientActivity
*/

#ifndef CLIENTACTIVITY_HPP_
#define CLIENTACTIVITY_HPP_

#include <chrono>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace rtype::server {

/**
 * @brief Thread-safe record of the last valid packet time of each client.
 *
 * Owned by the network side: it never touches the game world.
 */
class ClientActivity {
public:
    using Clock = std::chrono::steady_clock;

    /**
     * @brief Marks the client as active at now.
     */
    void touch(std::uint32_t clientId, Clock::time_point now);

    /**
     * @brief Stops tracking the client.
     *
     * @return true if the client was tracked.
     */
    bool forget(std::uint32_t clientId);

    /**
     * @brief Stops tracking and returns the clients silent for longer than
     * timeout.
     */
    [[nodiscard]] std::vector<std::uint32_t>
    takeExpired(Clock::time_point now, Clock::duration timeout);

private:
    std::mutex _mutex;
    std::unordered_map<std::uint32_t, Clock::time_point> _lastSeen;
};

} /* namespace rtype::server */

#endif /* !CLIENTACTIVITY_HPP_ */
