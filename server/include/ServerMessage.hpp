/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ServerMessage
*/

#ifndef SERVERMESSAGE_HPP_
#define SERVERMESSAGE_HPP_

#include <cstdint>
#include <optional>
#include <string>

#include "MessageQueue.hpp"
#include "Protocol/Protocol.hpp"

namespace rtype::server {

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
    std::optional<protocol::DisconnectReason> disconnect;
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

#endif /* !SERVERMESSAGE_HPP_ */
