/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientSession.hpp
*/

#ifndef CLIENTSESSION_HPP_
#define CLIENTSESSION_HPP_

#include <asio.hpp>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ClientHandshake.hpp"
#include "Connection.hpp"
#include "Messages.hpp"
#include "Timing.hpp"

namespace client::net {

/**
 * @brief The connection of the client to a server, over UDP.
 *
 * It wraps the socket, runs the handshake, sends the inputs, keeps the
 * reliable channel going and hands the received messages to the game. It is
 * driven by update(), called every frame: it never blocks.
 */
class ClientSession {
public:
    enum class State : std::uint8_t {
        CONNECTING,
        CONNECTED,
        REJECTED,
        FAILED,
        CLOSED
    };

    /**
     * @param host Name or address of the server.
     * @param port UDP port of the server.
     * @param name Player name, 1 to 16 printable characters.
     *
     * The state is FAILED if the host cannot be resolved.
     */
    ClientSession(const std::string &host, std::uint16_t port,
                  const std::string &name);

    /**
     * @brief Reads the socket, runs the handshake and the timers.
     */
    void update(rtype::protocol::TimePoint now);

    /**
     * @brief Sends the keys of one simulation step.
     *
     * The last 4 samples travel in every datagram, so a lost datagram loses
     * nothing.
     *
     * @return The id of the sample, 0 if the client is not connected.
     */
    std::uint16_t sendInput(std::uint8_t keys, rtype::protocol::TimePoint now);

    /**
     * @brief Tells the server the client leaves.
     */
    void disconnect(rtype::protocol::TimePoint now);

    /**
     * @brief Returns the messages received since the last call, in order.
     */
    [[nodiscard]] std::vector<rtype::protocol::Message> takeMessages();

    [[nodiscard]] State state() const noexcept;
    [[nodiscard]] const std::optional<rtype::protocol::Welcome> &
    welcome() const noexcept;
    [[nodiscard]] std::uint8_t rejectReason() const noexcept;
    [[nodiscard]] std::chrono::milliseconds rtt() const noexcept;

private:
    using Udp = asio::ip::udp;

    void receiveAll(rtype::protocol::TimePoint now);
    void handle(const std::string &datagram, rtype::protocol::TimePoint now);
    void deliver(const rtype::protocol::ReceivedDatagram &result);
    void runHandshake(rtype::protocol::TimePoint now);
    void runTimers(rtype::protocol::TimePoint now);
    void send(const std::string &datagram, rtype::protocol::TimePoint now);

    asio::io_context _context;
    Udp::socket _socket;
    Udp::endpoint _server;
    rtype::protocol::ClientHandshake _handshake;
    std::unique_ptr<rtype::protocol::Connection> _connection;
    State _state = State::CONNECTING;
    std::optional<rtype::protocol::Welcome> _welcome;
    std::vector<rtype::protocol::Message> _inbox;
    std::deque<std::uint8_t> _recentKeys;
    std::uint16_t _nextInputId = 1;
    rtype::protocol::TimePoint _lastSent;
    bool _ackDue = false;
};

} /* namespace client::net */

#endif /* !CLIENTSESSION_HPP_ */
