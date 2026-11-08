/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** Server
*/

#include "Server.hpp"

#include <array>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "PacketReader.hpp"

namespace rtype::server {

using protocol::DisconnectReason;
using protocol::MessageType;
using protocol::PacketReader;

using Nanosecond = std::chrono::nanoseconds;

namespace {

constexpr std::uint8_t SNAPSHOT_RATE = 20;
constexpr std::size_t MAX_DATAGRAMS_PER_PASS = 256;

std::string addressOf(const Udp::endpoint &endpoint)
{
    return endpoint.address().to_string() + "|" +
           std::to_string(endpoint.port());
}

std::optional<Udp::endpoint> endpointOf(const std::string &address)
{
    const std::size_t separator = address.rfind('|');
    std::error_code error;
    const auto ip = asio::ip::make_address(address.substr(0, separator), error);

    if (separator == std::string::npos || error) {
        return std::nullopt;
    }
    return Udp::endpoint(ip, static_cast<std::uint16_t>(
                                 std::stoi(address.substr(separator + 1))));
}

GatewayConfig gatewayConfigOf(const ServerConfig &config)
{
    GatewayConfig gateway;
    const std::uint32_t hz =
        std::clamp<std::uint32_t>(config.simulationHz, 1, 240);

    gateway.tickRate = static_cast<std::uint8_t>(hz);
    gateway.snapshotRate =
        static_cast<std::uint8_t>(std::min<std::uint32_t>(SNAPSHOT_RATE, hz));
    return gateway;
}

} // namespace

Server::Server(ServerConfig config)
    : _config(config),
      _incomingMessages(config.maxQueueCapacity),
      _outgoingMessages(config.maxQueueCapacity),
      _socket(_context, Udp::endpoint(Udp::v4(), config.port)),
      _gateway(
          gatewayConfigOf(config), protocol::CookieFactory::randomSecret(),
          [this](const std::string &address, const std::string &datagram) {
              sendDatagram(address, datagram);
          },
          [this](IncomingMessage message) {
              return receivePacket(std::move(message));
          },
          [this](std::uint32_t clientId, DisconnectReason reason) {
              _activity.forget(clientId);
              requestDisconnect(clientId, reason);
          })
{
    _socket.non_blocking(true);
}

Server::~Server()
{
    shutdown();
}

bool Server::start()
{
    bool expected = false;

    if (!_running.compare_exchange_strong(expected, true)) {
        return false;
    }

    _tickCount.store(0);

    _networkThread = std::jthread(
        [this](const std::stop_token &stopToken) { runNetwork(stopToken); });

    _simulationThread = std::jthread(
        [this](const std::stop_token &stopToken) { runSimulation(stopToken); });

    return true;
}

void Server::shutdown() noexcept
{
    if (!_running.exchange(false)) {
        return;
    }

    if (_networkThread.joinable()) {
        _networkThread.request_stop();
        _networkThread.join();
    }

    if (_simulationThread.joinable()) {
        _simulationThread.request_stop();
        _simulationThread.join();
    }
    _gateway.closeAll(DisconnectReason::SHUTDOWN, Clock::now());
}

bool Server::isRunning() const noexcept
{
    return _running.load();
}

std::uint64_t Server::tickCount() const noexcept
{
    return _tickCount.load();
}

Nanosecond Server::simulationInterval() const noexcept
{
    const std::uint32_t hz =
        _config.simulationHz > 0 ? _config.simulationHz : 60;

    return Nanosecond(std::chrono::seconds(1)) / hz;
}

void Server::runNetwork(const std::stop_token &stopToken)
{
    std::cout << "[server] Network thread started\n";
    while (!stopToken.stop_requested()) {
        receiveDatagrams();
        disconnectInactiveClients();
        sendOutgoingMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    std::cout << "[server] Network thread stopped\n";
}

void Server::receiveDatagrams()
{
    std::array<char, protocol::MAX_PACKET_SIZE + 1> buffer{};
    Udp::endpoint sender;
    std::error_code error;

    for (std::size_t i = 0; i < MAX_DATAGRAMS_PER_PASS; ++i) {
        const std::size_t size =
            _socket.receive_from(asio::buffer(buffer), sender, 0, error);

        if (error == asio::error::would_block ||
            error == asio::error::try_again) {
            return;
        }
        if (error) {
            continue;
        }
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch());

        _gateway.onDatagram(addressOf(sender),
                            std::string_view(buffer.data(), size), Clock::now(),
                            static_cast<std::uint64_t>(seconds.count()));
    }
}

void Server::sendDatagram(const std::string &address,
                          const std::string &datagram)
{
    const auto endpoint = endpointOf(address);
    std::error_code error;

    if (endpoint) {
        _socket.send_to(asio::buffer(datagram), *endpoint, 0, error);
    }
}

void Server::sendOutgoingMessages()
{
    while (auto message = _outgoingMessages.handleDequeue()) {
        _gateway.queueFromSimulation(*message);
    }
    _gateway.flush(Clock::now());
}

void Server::updateSimulationTick(Nanosecond timestep, Nanosecond &accumulator,
                                  const std::stop_token &stopToken)
{
    while (accumulator >= timestep && !stopToken.stop_requested()) {
        _tickCount.fetch_add(1);
        accumulator -= timestep;
    }
}

void Server::disconnectInactiveClients()
{
    const auto expired = _activity.takeExpired(ClientActivity::Clock::now(),
                                               _config.inactivityTimeout);

    for (const std::uint32_t clientId : expired) {
        _gateway.remove(clientId);
        requestDisconnect(clientId, DisconnectReason::TIMEOUT);
    }
}

void Server::requestDisconnect(std::uint32_t clientId, DisconnectReason reason)
{
    const bool queued = _incomingMessages.handleEnqueue(
        IncomingMessage{clientId, {}, reason, {}});

    if (!queued) {
        /* Tracked again, so the timeout retries once the queue drains. */
        _activity.touch(clientId, ClientActivity::Clock::now());
    }
}

void Server::handleGoodbye(std::uint32_t clientId)
{
    if (_activity.forget(clientId)) {
        requestDisconnect(clientId, DisconnectReason::QUIT);
    }
}

void Server::readIncomingMessage()
{
    while (auto message = _incomingMessages.handleDequeue()) {
        applyMessage(*message);
    }
    _playerCount.store(_players.size());
}

void Server::applyMessage(const IncomingMessage &message)
{
    if (message.disconnect) {
        removePlayer(message.clientId, *message.disconnect);
        return;
    }
    if (PacketReader::hasRecord(message.payload, MessageType::HELLO)) {
        if (message.slot) {
            _players.joinAt(message.clientId, *message.slot);
        } else {
            _players.join(message.clientId);
        }
    }
    /*
     * to do:
     * apply the other records of message.payload to the simulation state.
     */
}

void Server::removePlayer(std::uint32_t clientId, DisconnectReason reason)
{
    for (auto &notice : _players.leave(clientId, reason)) {
        _outgoingMessages.handleEnqueue(std::move(notice));
    }
}

bool Server::receivePacket(IncomingMessage message)
{
    if (!PacketReader::isValid(message.payload)) {
        _rejectedPackets.fetch_add(1);
        return false;
    }
    if (PacketReader::hasRecord(message.payload, MessageType::GOODBYE)) {
        handleGoodbye(message.clientId);
        return true;
    }
    _activity.touch(message.clientId, ClientActivity::Clock::now());
    return _incomingMessages.handleEnqueue(std::move(message));
}

std::uint64_t Server::rejectedPackets() const noexcept
{
    return _rejectedPackets.load();
}

std::uint16_t Server::port() const
{
    return _socket.local_endpoint().port();
}

std::size_t Server::playerCount() const noexcept
{
    return _playerCount.load();
}

void Server::runSimulationLoop(const std::stop_token &stopToken,
                               Nanosecond timestep)
{
    using Clock = std::chrono::steady_clock;
    const auto maxFrameTime =
        std::chrono::duration_cast<Clock::duration>(_config.maxFrameTime);
    auto previousTime = Clock::now();
    Nanosecond accumulator = Nanosecond::zero();

    while (!stopToken.stop_requested()) {
        const auto currentTime = Clock::now();
        const auto frameTime =
            std::min(currentTime - previousTime, maxFrameTime);
        previousTime = currentTime;
        accumulator += std::chrono::duration_cast<Nanosecond>(frameTime);
        readIncomingMessage();
        updateSimulationTick(timestep, accumulator, stopToken);
        std::this_thread::sleep_until(previousTime + timestep);
    }
}

void Server::runSimulation(const std::stop_token &stopToken)
{
    const auto timestep = simulationInterval();

    std::cout << "[server] Simulation thread started at "
              << _config.simulationHz << " Hz\n";

    runSimulationLoop(stopToken, timestep);

    std::cout << "[server] Simulation thread stopped after "
              << _tickCount.load() << " ticks\n";
}

} /* namespace rtype::server */
