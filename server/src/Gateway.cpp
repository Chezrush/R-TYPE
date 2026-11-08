/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Gateway.cpp
*/

#include "Gateway.hpp"

#include <algorithm>
#include <array>
#include <utility>
#include <variant>

#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"

namespace rtype::server {

using protocol::DatagramStatus;
using protocol::DisconnectReason;
using protocol::Message;
using protocol::MessageCodec;
using protocol::PacketReader;
using protocol::ServerHandshake;

namespace {

constexpr double DATAGRAMS_PER_SECOND = 200.0;
constexpr std::size_t MAX_TRACKED_ADDRESSES = 1024;
constexpr unsigned MAX_INVALID_DATAGRAMS = 50;
constexpr auto INVALID_WINDOW = std::chrono::seconds(10);

std::string helloDatagram(const protocol::Hello &hello)
{
    std::string packet;

    protocol::PacketBuilder::writeHeader(packet, protocol::MessageHeader{});
    MessageCodec::appendMessage(packet, hello);
    return packet;
}

bool hasGoodbye(const protocol::ReceivedDatagram &result)
{
    return std::any_of(result.unreliable.begin(), result.unreliable.end(),
                       [](const Message &message) {
                           return std::holds_alternative<protocol::Goodbye>(
                               message);
                       });
}

} // namespace

Gateway::Gateway(GatewayConfig config, const protocol::SipKey &secret,
                 Send send, Deliver deliver, Drop drop)
    : _config(config),
      _send(std::move(send)),
      _deliver(std::move(deliver)),
      _drop(std::move(drop)),
      _handshake(secret),
      _datagramLimiter(DATAGRAMS_PER_SECOND, DATAGRAMS_PER_SECOND,
                       MAX_TRACKED_ADDRESSES)
{}

std::size_t Gateway::clientCount() const
{
    const std::lock_guard lock(_mutex);

    return _byAddress.size();
}

Gateway::Remote *Gateway::find(std::uint32_t clientId)
{
    const auto found = _byId.find(clientId);

    return found == _byId.end() ? nullptr : found->second;
}

std::uint8_t Gateway::freeSlot() const
{
    std::array<bool, protocol::MAX_PLAYERS> taken{};

    for (const auto &[address, remote] : _byAddress) {
        taken[remote->slot] = true;
    }
    return static_cast<std::uint8_t>(
        std::find(taken.begin(), taken.end(), false) - taken.begin());
}

std::string Gateway::goodbyeDatagram(Remote &remote, DisconnectReason reason,
                                     TimePoint now)
{
    const std::vector<Message> goodbye = {
        protocol::Goodbye{static_cast<std::uint8_t>(reason)}};

    return remote.connection.buildDatagram(now, goodbye);
}

void Gateway::onDatagram(const std::string &address, std::string_view datagram,
                         TimePoint now, std::uint64_t nowSeconds)
{
    const std::lock_guard lock(_mutex);
    const auto known = _byAddress.find(address);

    if (known == _byAddress.end()) {
        onStranger(address, datagram, now, nowSeconds);
        return;
    }
    if (_datagramLimiter.allow(address, now)) {
        onClient(*known->second, datagram, now, nowSeconds);
    }
}

void Gateway::onStranger(const std::string &address, std::string_view datagram,
                         TimePoint now, std::uint64_t nowSeconds)
{
    const bool room = _byAddress.size() < _config.maxPlayers;
    const auto decision =
        _handshake.handle(address, datagram, now, nowSeconds, room);

    if (decision.kind == ServerHandshake::Kind::REPLY) {
        _send(address, decision.reply);
    } else if (decision.kind == ServerHandshake::Kind::ACCEPT) {
        accept(address, decision.hello, now);
    }
}

void Gateway::onClient(Remote &remote, std::string_view datagram, TimePoint now,
                       std::uint64_t nowSeconds)
{
    const auto header = PacketReader::parseHeader(datagram);

    if (header && header->session == 0) {
        const auto decision =
            _handshake.handle(remote.address, datagram, now, nowSeconds, true);
        if (decision.kind == ServerHandshake::Kind::REPLY) {
            _send(remote.address, decision.reply);
        } else if (decision.kind == ServerHandshake::Kind::ACCEPT) {
            const std::string address = remote.address;
            const bool again = decision.hello.cookie == remote.cookie;

            if (again) {
                sendWelcome(remote, now);
                return;
            }
            close(remote, DisconnectReason::QUIT, now, true);
            accept(address, decision.hello, now);
        }
        return;
    }
    onResult(remote, remote.connection.receive(datagram, now), datagram, now);
}

void Gateway::onResult(Remote &remote, const protocol::ReceivedDatagram &result,
                       std::string_view datagram, TimePoint now)
{
    if (result.status == DatagramStatus::INVALID ||
        result.status == DatagramStatus::RELIABLE_OUT_OF_WINDOW) {
        countInvalid(remote, now);
        return;
    }
    if (result.status != DatagramStatus::ACCEPTED) {
        return;
    }
    remote.ackDue = remote.ackDue || result.hadRecords;
    const std::uint32_t clientId = remote.clientId;
    const bool leaving = hasGoodbye(result);

    if (leaving) {
        close(remote, DisconnectReason::QUIT, now, false);
    }
    _deliver(IncomingMessage{clientId, std::string(datagram), {}, {}});
}

void Gateway::countInvalid(Remote &remote, TimePoint now)
{
    if (remote.invalidCount == 0 ||
        now - remote.invalidSince > INVALID_WINDOW) {
        remote.invalidSince = now;
        remote.invalidCount = 0;
    }
    if (++remote.invalidCount > MAX_INVALID_DATAGRAMS) {
        close(remote, DisconnectReason::PROTOCOL, now, true);
    }
}

void Gateway::accept(const std::string &address, const protocol::Hello &hello,
                     TimePoint now)
{
    auto remote = std::make_unique<Remote>();

    remote->clientId = _nextClientId++;
    remote->slot = freeSlot();
    remote->cookie = hello.cookie;
    remote->address = address;
    remote->name = hello.name;
    remote->connection = protocol::Connection(ServerHandshake::newSession());
    remote->lastSent = now;
    Remote &ref = *remote;

    _byId[ref.clientId] = &ref;
    _byAddress[address] = std::move(remote);
    introduce(ref);
    sendWelcome(ref, now);
    _deliver(IncomingMessage{ref.clientId, helloDatagram(hello), {}, ref.slot});
}

void Gateway::introduce(Remote &newcomer)
{
    const Message joined = protocol::PlayerJoined{newcomer.slot, newcomer.name};

    for (const auto &[address, other] : _byAddress) {
        if (other.get() == &newcomer) {
            continue;
        }
        newcomer.connection.queueReliable(
            protocol::PlayerJoined{other->slot, other->name});
        other->connection.queueReliable(joined);
    }
}

void Gateway::sendWelcome(Remote &remote, TimePoint now)
{
    protocol::Welcome welcome;

    welcome.playerIndex = remote.slot;
    welcome.maxPlayers = _config.maxPlayers;
    welcome.tickRate = _config.tickRate;
    welcome.snapshotRate = _config.snapshotRate;
    welcome.worldWidth = _config.worldWidth;
    welcome.worldHeight = _config.worldHeight;
    welcome.shipSpeed = _config.shipSpeed;
    remote.unreliable.push_back(welcome);
    sendNow(remote, now);
}

void Gateway::sendNow(Remote &remote, TimePoint now)
{
    _send(remote.address,
          remote.connection.buildDatagram(now, remote.unreliable));
    remote.unreliable.clear();
    remote.lastSent = now;
    remote.ackDue = false;
}

void Gateway::queueFromSimulation(const OutgoingMessage &message)
{
    const std::lock_guard lock(_mutex);
    Remote *remote = find(message.clientId);
    std::string_view records = PacketReader::recordsOf(message.payload);

    while (remote != nullptr) {
        const auto record = PacketReader::readRecord(records);

        if (!record) {
            return;
        }
        const bool reliable = protocol::isReliable(record->type);
        const auto body = reliable ? record->body.substr(protocol::REL_ID_SIZE)
                                   : record->body;
        auto decoded = MessageCodec::decodeBody(record->type, body);

        if (!decoded) {
            continue;
        }
        if (!reliable) {
            remote->unreliable.push_back(std::move(*decoded));
        } else if (!remote->connection.queueReliable(*decoded)) {
            close(*remote, DisconnectReason::QUEUE_OVERFLOW, Clock::now(),
                  true);
            return;
        }
    }
}

void Gateway::flush(TimePoint now)
{
    const std::lock_guard lock(_mutex);

    for (const auto &[address, remote] : _byAddress) {
        const bool due = remote->connection.hasDueReliable(now) ||
                         !remote->unreliable.empty() || remote->ackDue ||
                         now - remote->lastSent >= _config.keepalive;

        if (due) {
            sendNow(*remote, now);
        }
    }
}

void Gateway::close(Remote &remote, DisconnectReason reason, TimePoint now,
                    bool notifySimulation)
{
    const std::uint32_t clientId = remote.clientId;
    const std::string address = remote.address;

    if (notifySimulation) {
        _send(address, goodbyeDatagram(remote, reason, now));
        _drop(clientId, reason);
    }
    _byId.erase(clientId);
    _byAddress.erase(address);
}

void Gateway::remove(std::uint32_t clientId)
{
    const std::lock_guard lock(_mutex);
    const Remote *remote = find(clientId);

    if (remote != nullptr) {
        const std::string address = remote->address;

        _byId.erase(clientId);
        _byAddress.erase(address);
    }
}

void Gateway::closeAll(DisconnectReason reason, TimePoint now)
{
    const std::lock_guard lock(_mutex);

    for (const auto &[address, remote] : _byAddress) {
        _send(address, goodbyeDatagram(*remote, reason, now));
    }
    _byId.clear();
    _byAddress.clear();
}

} /* namespace rtype::server */
