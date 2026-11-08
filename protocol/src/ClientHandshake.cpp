/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientHandshake.cpp
*/

#include "ClientHandshake.hpp"

#include <utility>

#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"

namespace rtype::protocol {

ClientHandshake::ClientHandshake(std::string name, TimePoint start)
    : _name(std::move(name)),
      _start(start),
      _lastSent(start)
{}

std::string ClientHandshake::buildHello()
{
    std::string packet;
    MessageHeader header;

    header.seq = _seq++;
    PacketBuilder::writeHeader(packet, header);
    MessageCodec::appendMessage(packet, Hello{_cookie, _name});
    return packet;
}

std::optional<std::string> ClientHandshake::poll(TimePoint now)
{
    if (_state != State::CONNECTING) {
        return std::nullopt;
    }
    if (now - _start >= GIVE_UP_DELAY) {
        _state = State::FAILED;
        return std::nullopt;
    }
    if (_sentOnce && !_sendNow && now - _lastSent < RETRY_DELAY) {
        return std::nullopt;
    }
    _sentOnce = true;
    _sendNow = false;
    _lastSent = now;
    return buildHello();
}

void ClientHandshake::apply(const Message &message, std::uint32_t session)
{
    const auto *challenge = std::get_if<Challenge>(&message);
    const auto *reject = std::get_if<Reject>(&message);
    const auto *welcome = std::get_if<Welcome>(&message);

    if (challenge != nullptr && session == 0) {
        _cookie = challenge->cookie;
        _sendNow = true;
    } else if (reject != nullptr && session == 0) {
        _rejectReason = reject->reason;
        _state = State::REJECTED;
    } else if (welcome != nullptr && session != 0) {
        _welcome = *welcome;
        _session = session;
        _state = State::CONNECTED;
    }
}

void ClientHandshake::handleRecords(std::string_view datagram,
                                    std::uint32_t session)
{
    std::string_view remaining = PacketReader::recordsOf(datagram);

    while (const auto record = PacketReader::readRecord(remaining)) {
        const auto message =
            MessageCodec::decodeBody(record->type, record->body);

        if (message) {
            apply(*message, session);
        }
        if (_state != State::CONNECTING) {
            return;
        }
    }
}

void ClientHandshake::onDatagram(std::string_view datagram)
{
    if (_state != State::CONNECTING) {
        return;
    }
    const auto peeked = PacketReader::peekHeader(datagram);

    if (peeked && peeked->magicNumber == PROTOCOL_MAGIC &&
        peeked->version != PROTOCOL_VERSION) {
        _serverVersion = peeked->version;
        _rejectReason = static_cast<std::uint8_t>(RejectReason::VERSION);
        _state = State::REJECTED;
        return;
    }
    const auto header = PacketReader::parseHeader(datagram);

    if (header && PacketReader::isValid(datagram)) {
        handleRecords(datagram, header->session);
    }
}

ClientHandshake::State ClientHandshake::state() const noexcept
{
    return _state;
}

const std::optional<Welcome> &ClientHandshake::welcome() const noexcept
{
    return _welcome;
}

std::uint32_t ClientHandshake::session() const noexcept
{
    return _session;
}

std::uint8_t ClientHandshake::rejectReason() const noexcept
{
    return _rejectReason;
}

std::uint8_t ClientHandshake::serverVersion() const noexcept
{
    return _serverVersion;
}

} /* namespace rtype::protocol */
