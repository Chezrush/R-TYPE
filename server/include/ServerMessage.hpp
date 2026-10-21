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