/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectionEvents.hpp
*/

#ifndef CONNECTIONEVENTS_HPP_
#define CONNECTIONEVENTS_HPP_

#include <cstdint>
#include <string>

namespace client::events {

/**
 * @brief Why a connection failed.
 *
 * The first values are the protocol `reject` reasons. The others are found by
 * the client itself. The enumeration is open: an unknown value is kept as is.
 */
enum class ConnectionFailure : std::uint8_t {
    SERVER_FULL = 1,
    INVALID_NAME = 2,
    VERSION = 3,
    RATE_LIMITED = 4,
    SHUTTING_DOWN = 5,
    TIMEOUT = 0xF0,
    INVALID_ADDRESS = 0xF1
};

/** @brief The player asked to join a server. */
struct ConnectRequested {
    std::string host;
    std::uint16_t port = 0;
    std::string nickname;
};

/** @brief The player gave up on the connection in progress. */
struct ConnectCancelled {};

/** @brief The network layer started the handshake. */
struct ConnectionStarted {
    std::string host;
    std::uint16_t port = 0;
};

/** @brief The connection broke during the game: the server stopped answering.
 */
struct ConnectionLost {};

/** @brief The network layer could not connect. */
struct ConnectionFailed {
    ConnectionFailure reason = ConnectionFailure::TIMEOUT;
};

} // namespace client::events

#endif /* !CONNECTIONEVENTS_HPP_ */
