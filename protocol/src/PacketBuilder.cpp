/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** PacketBuilder.cpp
*/

#include "PacketBuilder.hpp"

#include "ByteOrder.hpp"

namespace rtype::protocol {

void PacketBuilder::writeHeader(std::string &packet,
                                const MessageHeader &header)
{
    writeUint16BigEndian(packet, header.magicNumber);
    writeUint8(packet, header.version);
    writeUint8(packet, header.flags);
    writeUint32BigEndian(packet, header.session);
    writeUint16BigEndian(packet, header.seq);
    writeUint16BigEndian(packet, header.ack);
    writeUint32BigEndian(packet, header.ackBits);
}

void PacketBuilder::appendRecord(std::string &packet, MessageType type,
                                 std::string_view body)
{
    writeUint8(packet, static_cast<std::uint8_t>(type));
    writeUint16BigEndian(packet, static_cast<std::uint16_t>(body.size()));
    packet += body;
}

void PacketBuilder::appendHello(std::string &packet, std::uint64_t cookie,
                                std::string_view name)
{
    std::string body;

    writeUint32BigEndian(body, static_cast<std::uint32_t>(cookie >> 32U));
    writeUint32BigEndian(body, static_cast<std::uint32_t>(cookie));
    writeUint8(body, static_cast<std::uint8_t>(name.size()));
    body += name;
    appendRecord(packet, MessageType::HELLO, body);
}

void PacketBuilder::appendGoodbye(std::string &packet, DisconnectReason reason)
{
    std::string body;

    writeUint8(body, static_cast<std::uint8_t>(reason));
    appendRecord(packet, MessageType::GOODBYE, body);
}

void PacketBuilder::appendPlayerLeft(std::string &packet, std::uint16_t relId,
                                     std::uint8_t playerIndex,
                                     DisconnectReason reason)
{
    std::string body;

    writeUint16BigEndian(body, relId);
    writeUint8(body, playerIndex);
    writeUint8(body, static_cast<std::uint8_t>(reason));
    appendRecord(packet, MessageType::PLAYERLEFT, body);
}

} /* namespace rtype::protocol */
