/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ServerTestSupport
*/

#ifndef SERVERTESTSUPPORT_HPP_
#define SERVERTESTSUPPORT_HPP_

#include <chrono>
#include <functional>
#include <thread>

#include "Server.hpp"

/**
 * @brief Configuration of a test server on a free port.
 */
inline rtype::server::ServerConfig testConfig(std::chrono::milliseconds timeout)
{
    rtype::server::ServerConfig config;

    config.port = 0;
    config.inactivityTimeout = timeout;
    return config;
}

/**
 * @brief Polls the condition until it holds or three seconds elapsed.
 */
inline bool waitFor(const std::function<bool()> &condition)
{
    using namespace std::chrono_literals;
    const auto deadline = std::chrono::steady_clock::now() + 3s;

    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::sleep_for(5ms);
    }
    return true;
}

#endif /* !SERVERTESTSUPPORT_HPP_ */
