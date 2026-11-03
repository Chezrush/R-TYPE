/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ServerMessage
*/

#ifndef SERVERMESSAGES_HPP_
#define SERVERMESSAGES_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "MessageQueue.hpp"

namespace rtype::server {

inline constexpr std::size_t PACKET_HEADER_SIZE = 16;
inline constexpr std::size_t RECORD_PREFIX_SIZE = 3;
inline constexpr std::size_t REL_ID_SIZE = 2;
inline constexpr std::size_t MAX_PACKET_SIZE = 1200;
inline constexpr std::size_t MAX_RECORDS = 64;
inline constexpr std::size_t MAX_NAME_LENGTH = 16;
inline constexpr std::size_t MAX_INPUTS = 8;
inline constexpr std::size_t MAX_SNAPSHOT_ENTITIES = 128;
inline constexpr std::uint16_t PROTOCOL_MAGIC = 0x5254;
inline constexpr std::uint8_t PROTOCOL_VERSION = 1;
inline constexpr std::uint8_t MAX_PLAYERS = 16;
inline constexpr std::uint8_t NO_PLAYER = 0xFF;
inline constexpr std::uint8_t FLAG_ACK_VALID = 0x01;
inline constexpr std::uint8_t RELIABLE_BIT = 0x80;
inline constexpr std::uint8_t KEYS_MASK = 0x1F;

/**
 * @brief Values of goodbye.reason, also used by player_left.reason.
 */
enum class DisconnectReason : std::uint8_t {
    QUIT = 0,
    TIMEOUT = 1,
    SHUTDOWN = 2,
    PROTOCOL = 3,
    QUEUE_OVERFLOW = 4
};

/**
 * @brief Command received from the network and processed by the simulation.
 *
 * The payload owns its data: it contains no reference or pointer to a
 * mutable game entity. When disconnect is set, the message carries no
 * payload and asks the simulation to remove the client.
 */
struct IncomingMessage {
    std::uint32_t clientId = 0;
    std::string payload;
    std::optional<DisconnectReason> disconnect;
};

enum class MessageType : std::uint8_t {
    HELLO = 0x01,
    CHALLENGE = 0x02,
    WELCOME = 0x03,
    REJECT = 0x04,
    GOODBYE = 0x05,
    INPUT = 0x10,
    SNAPSHOT = 0x11,
    SPAWN = 0x81,
    DESTROY = 0x82,
    PLAYERJOINED = 0x83,
    PLAYERLEFT = 0x84,
    SCORE = 0x85,
    GAMESTATE = 0x86
};

struct MessageHeader {
    std::uint16_t magicNumber = 0;
    std::uint8_t version = 0;
    std::uint8_t flags = 0;
    std::uint32_t session = 0;
    std::uint16_t seq = 0;
    std::uint16_t ack = 0;
    std::uint32_t ackBits = 0;
};

/**
 * @brief Message produced by the simulation and sent by the network worker.
 *
 * The payload owns its data and can safely outlive the simulation tick
 * that created it.
 */
struct OutgoingMessage {
    std::uint32_t clientId = 0;
    std::string payload;
};

using IncomingQueue = MessageQueue<IncomingMessage>;
using OutgoingQueue = MessageQueue<OutgoingMessage>;

} /* namespace rtype::server */

#endif /* !SERVERMESSAGES_HPP_ */
