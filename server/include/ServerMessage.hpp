/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ServerMessage
*/

#ifndef SERVERMESSAGES_HPP_
#define SERVERMESSAGES_HPP_

#include <cstdint>
#include <string>

#include "MessageQueue.hpp"

namespace rtype::server {

inline constexpr std::size_t PACKET_HEADER_SIZE = 6;
inline constexpr std::uint16_t PROTOCOL_MAGIC = 0x5254;
inline constexpr std::uint8_t PROTOCOL_VERSION = 1;
inline constexpr int MAX_MESSAGE_TYPE = 8;


/**
 * @brief Command received from the network and processed by the simulation.
 *
 * The payload owns its data: it contains no reference or pointer to a
 * mutable game entity.
 */
struct IncomingMessage {
    std::uint32_t clientId = 0;
    std::string payload;
};

enum class MessageType : uint8_t {
    HELLO = 1,
    WELCOME = 2,
    GOODBYE = 3,
    INPUT = 4,
    ENTITYSPAWN = 5,
    ENTITYSTATE = 6,
    ENTITYDESTROY = 7,
    PLAYERLEFT = 8
};

struct MessageHeader {
    std::uint16_t magicNumber;
    std::uint8_t version;
    MessageType type = HELLO;
    std::uint16_t size;
};

struct MessageBody {

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
