/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientSession.cpp
*/

#include "Client/Net/ClientSession.hpp"

#include <array>
#include <utility>
#include <variant>

namespace client::net {

using rtype::protocol::ClientHandshake;
using rtype::protocol::Connection;
using rtype::protocol::Message;
using rtype::protocol::TimePoint;

namespace {

constexpr std::size_t INPUTS_PER_DATAGRAM = 4;
constexpr auto KEEPALIVE = std::chrono::seconds(1);
constexpr auto SILENCE_LIMIT = std::chrono::seconds(5);

} // namespace

ClientSession::ClientSession(const std::string &host, std::uint16_t port,
                             const std::string &name)
    : _socket(_context),
      _handshake(name, rtype::protocol::Clock::now()),
      _lastSent(rtype::protocol::Clock::now())
{
    std::error_code error;
    Udp::resolver resolver(_context);
    const auto found =
        resolver.resolve(Udp::v4(), host, std::to_string(port), error);

    if (error || found.empty()) {
        _state = State::FAILED;
        return;
    }
    _server = *found.begin();
    _socket.open(Udp::v4(), error);
    _socket.non_blocking(true, error);
    if (error) {
        _state = State::FAILED;
    }
}

void ClientSession::send(const std::string &datagram, TimePoint now)
{
    std::error_code error;

    _socket.send_to(asio::buffer(datagram), _server, 0, error);
    _lastSent = now;
}

void ClientSession::deliver(const rtype::protocol::ReceivedDatagram &result)
{
    _inbox.insert(_inbox.end(), result.reliable.begin(), result.reliable.end());
    for (const Message &message : result.unreliable) {
        if (const auto *welcome =
                std::get_if<rtype::protocol::Welcome>(&message)) {
            _welcome = *welcome;
        } else if (std::holds_alternative<rtype::protocol::Goodbye>(message)) {
            _state = State::CLOSED;
        } else {
            _inbox.push_back(message);
        }
    }
    _ackDue = _ackDue || result.hadRecords;
}

void ClientSession::handle(const std::string &datagram, TimePoint now)
{
    if (!_connection) {
        _handshake.onDatagram(datagram);
        if (_handshake.state() == ClientHandshake::State::CONNECTED) {
            _connection = std::make_unique<Connection>(_handshake.session());
            _state = State::CONNECTED;
        }
    }
    if (_connection) {
        deliver(_connection->receive(datagram, now));
    }
}

void ClientSession::receiveAll(TimePoint now)
{
    std::array<char, rtype::protocol::MAX_PACKET_SIZE + 1> buffer{};
    Udp::endpoint from;
    std::error_code error;

    for (;;) {
        const std::size_t size =
            _socket.receive_from(asio::buffer(buffer), from, 0, error);

        if (error == asio::error::would_block ||
            error == asio::error::try_again) {
            return;
        }
        if (!error && from == _server) {
            handle(std::string(buffer.data(), size), now);
        }
    }
}

void ClientSession::runHandshake(TimePoint now)
{
    if (const auto hello = _handshake.poll(now)) {
        send(*hello, now);
    }
    if (_handshake.state() == ClientHandshake::State::REJECTED) {
        _state = State::REJECTED;
    } else if (_handshake.state() == ClientHandshake::State::FAILED) {
        _state = State::FAILED;
    }
}

void ClientSession::runTimers(TimePoint now)
{
    if (now - _connection->lastReceived() > SILENCE_LIMIT) {
        _state = State::FAILED;
        return;
    }
    const bool due = _connection->hasDueReliable(now) || _ackDue ||
                     now - _lastSent >= KEEPALIVE;

    if (due) {
        send(_connection->buildDatagram(now), now);
        _ackDue = false;
    }
}

void ClientSession::update(TimePoint now)
{
    if (_state == State::FAILED || _state == State::REJECTED ||
        _state == State::CLOSED) {
        return;
    }
    receiveAll(now);
    if (_state == State::CONNECTING) {
        runHandshake(now);
    } else if (_state == State::CONNECTED) {
        runTimers(now);
    }
}

std::uint16_t ClientSession::sendInput(std::uint8_t keys, TimePoint now)
{
    if (_state != State::CONNECTED) {
        return 0;
    }
    _recentKeys.push_back(keys);
    if (_recentKeys.size() > INPUTS_PER_DATAGRAM) {
        _recentKeys.pop_front();
    }
    const std::uint16_t id = _nextInputId++;
    const auto first = static_cast<std::uint16_t>(id - _recentKeys.size() + 1);
    const std::vector<Message> input = {rtype::protocol::Input{
        first, {_recentKeys.begin(), _recentKeys.end()}}};

    send(_connection->buildDatagram(now, input), now);
    _ackDue = false;
    return id;
}

void ClientSession::disconnect(TimePoint now)
{
    if (_state != State::CONNECTED) {
        return;
    }
    const std::vector<Message> goodbye = {rtype::protocol::Goodbye{
        static_cast<std::uint8_t>(rtype::protocol::DisconnectReason::QUIT)}};

    send(_connection->buildDatagram(now, goodbye), now);
    _state = State::CLOSED;
}

std::vector<Message> ClientSession::takeMessages()
{
    return std::exchange(_inbox, {});
}

ClientSession::State ClientSession::state() const noexcept
{
    return _state;
}

const std::optional<rtype::protocol::Welcome> &
ClientSession::welcome() const noexcept
{
    return _welcome;
}

std::uint8_t ClientSession::rejectReason() const noexcept
{
    return _handshake.rejectReason();
}

std::chrono::milliseconds ClientSession::rtt() const noexcept
{
    if (!_connection) {
        return std::chrono::milliseconds(0);
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        _connection->rtt().smoothed());
}

} /* namespace client::net */
