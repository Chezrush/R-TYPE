/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PacketBuilder.hpp
*/

#ifndef PACKETBUILDER_HPP_
#define PACKETBUILDER_HPP_

#include <cstdint>
#include <string>
#include <string_view>

#include "Protocol.hpp"

namespace rtype::protocol {

class PacketBuilder {
public:
    static void writeHeader(std::string &packet, const MessageHeader &header);

    /**
     * @brief Appends a record. The body of a reliable record must start with
     * its rel_id.
     */
    static void appendRecord(std::string &packet, MessageType type,
                             std::string_view body);

    static void appendHello(std::string &packet, std::uint64_t cookie,
                            std::string_view name);
    static void appendGoodbye(std::string &packet, DisconnectReason reason);
    static void appendPlayerLeft(std::string &packet, std::uint16_t relId,
                                 std::uint8_t playerIndex,
                                 DisconnectReason reason);
};

} /* namespace rtype::protocol */

#endif /* !PACKETBUILDER_HPP_ */
