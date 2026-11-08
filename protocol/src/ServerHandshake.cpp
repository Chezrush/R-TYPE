/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ServerHandshake.cpp
*/

#include "ServerHandshake.hpp"

#include <random>
#include <utility>

#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"

namespace rtype::protocol {

namespace {

constexpr double HELLOS_PER_SECOND = 5.0;
constexpr double HELLO_BURST = 5.0;
constexpr std::size_t MAX_TRACKED_ADDRESSES = 4096;
constexpr std::size_t MIN_VERSION_PROBE_SIZE = 20;

std::string replyDatagram(const Message &message)
{
    std::string packet;

    PacketBuilder::writeHeader(packet, MessageHeader{});
    MessageCodec::appendMessage(packet, message);
    return packet;
}

std::optional<Hello> parseHello(std::string_view datagram)
{
    const auto header = PacketReader::parseHeader(datagram);

    if (!header || header->session != 0 || !PacketReader::isValid(datagram)) {
        return std::nullopt;
    }
    std::string_view records = PacketReader::recordsOf(datagram);
    const auto record = PacketReader::readRecord(records);
    const auto message =
        record ? MessageCodec::decodeBody(record->type, record->body)
               : std::nullopt;

    if (!records.empty() || !message ||
        !std::holds_alternative<Hello>(*message)) {
        return std::nullopt;
    }
    return std::get<Hello>(*message);
}

} // namespace

ServerHandshake::ServerHandshake(const SipKey &secret)
    : _cookies(secret),
      _helloLimiter(HELLOS_PER_SECOND, HELLO_BURST, MAX_TRACKED_ADDRESSES),
      _versionLimiter(1.0, 1.0, MAX_TRACKED_ADDRESSES)
{}

std::uint32_t ServerHandshake::newSession()
{
    std::random_device device;
    std::uint32_t session = 0;

    while (session == 0) {
        session = device();
    }
    return session;
}

ServerHandshake::Decision ServerHandshake::reject(RejectReason reason)
{
    return {Kind::REPLY,
            replyDatagram(Reject{static_cast<std::uint8_t>(reason)}),
            {}};
}

ServerHandshake::Decision
ServerHandshake::onWrongVersion(std::string_view address,
                                std::string_view datagram, TimePoint now)
{
    if (datagram.size() < MIN_VERSION_PROBE_SIZE ||
        !_versionLimiter.allow(address, now)) {
        return {};
    }
    return reject(RejectReason::VERSION);
}

ServerHandshake::Decision ServerHandshake::onHello(std::string_view address,
                                                   const Hello &hello,
                                                   TimePoint now,
                                                   std::uint64_t nowSeconds,
                                                   bool slotAvailable)
{
    if (!_helloLimiter.allow(address, now)) {
        return {};
    }
    if (!_cookies.verify(address, hello.cookie, nowSeconds)) {
        const Challenge challenge{_cookies.make(address, nowSeconds)};

        return {Kind::REPLY, replyDatagram(challenge), {}};
    }
    if (hello.name.empty()) {
        return reject(RejectReason::INVALID_NAME);
    }
    if (!slotAvailable) {
        return reject(RejectReason::SERVER_FULL);
    }
    return {Kind::ACCEPT, {}, hello};
}

ServerHandshake::Decision ServerHandshake::handle(std::string_view address,
                                                  std::string_view datagram,
                                                  TimePoint now,
                                                  std::uint64_t nowSeconds,
                                                  bool slotAvailable)
{
    const auto peeked = PacketReader::peekHeader(datagram);

    if (peeked && peeked->magicNumber == PROTOCOL_MAGIC &&
        peeked->version != PROTOCOL_VERSION) {
        return onWrongVersion(address, datagram, now);
    }
    const auto hello = parseHello(datagram);

    if (!hello) {
        return {};
    }
    return onHello(address, *hello, now, nowSeconds, slotAvailable);
}

} /* namespace rtype::protocol */
