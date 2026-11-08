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
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "MessageCodec.hpp"
#include "PacketReader.hpp"

namespace rtype::server {

using protocol::DisconnectReason;
using protocol::MessageType;
using protocol::PacketReader;

using Nanosecond = std::chrono::nanoseconds;

namespace {

constexpr std::uint8_t SNAPSHOT_RATE = 20;
constexpr std::uint32_t MAX_TICK_RATE = 240;
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

game::GameRules rulesOf(const ServerConfig &config)
{
    game::GameRules rules;

    rules.tickRate =
        std::clamp<std::uint32_t>(config.simulationHz, 1, MAX_TICK_RATE);
    return rules;
}

/**
 * @brief The welcome sent to the clients describes the rules of the game.
 */
GatewayConfig gatewayConfigOf(const game::GameRules &rules)
{
    GatewayConfig gateway;

    gateway.maxPlayers = rules.maxPlayers;
    gateway.tickRate = static_cast<std::uint8_t>(rules.tickRate);
    gateway.snapshotRate = static_cast<std::uint8_t>(
        std::min<std::uint32_t>(SNAPSHOT_RATE, rules.tickRate));
    gateway.worldWidth = static_cast<std::uint16_t>(rules.worldWidth);
    gateway.worldHeight = static_cast<std::uint16_t>(rules.worldHeight);
    gateway.shipSpeed = static_cast<std::uint16_t>(rules.shipSpeed);
    return gateway;
}

} // namespace

Server::Server(ServerConfig config)
    : _config(config),
      _bridge(rulesOf(config), std::random_device{}(), SNAPSHOT_RATE),
      _players(_bridge.rules().maxPlayers),
      _incomingMessages(config.maxQueueCapacity),
      _outgoingMessages(config.maxQueueCapacity),
      _socket(_context, Udp::endpoint(Udp::v4(), config.port)),
      _gateway(
          gatewayConfigOf(_bridge.rules()),
          protocol::CookieFactory::randomSecret(),
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
    return Nanosecond(std::chrono::seconds(1)) / _bridge.rules().tickRate;
}

void Server::runNetwork(const std::stop_token &stopToken)
{
    std::cout << "[server] Network thread started\n";
    while (!stopToken.stop_requested()) {
        receiveDatagrams();
        disconnectInactiveClients();
        sendOutgoingMessages();
        printStats();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    std::cout << "[server] Network thread stopped\n";
}

void Server::printStats()
{
    const auto now = Clock::now();

    if (!_config.showStats || now - _lastStatsLine < _config.statsInterval) {
        return;
    }
    _lastStatsLine = now;
    const GatewayStats stats = _gateway.stats();
    const NetRates rates = _rateMeter.sample(stats.totals, now);

    std::cout << formatNetLine(stats.clients.size(), rates,
                               stats.totals.dropped, stats.averageRtt())
              << "\n"
              << std::flush;
}

void Server::dispatch(const Udp::endpoint &sender, std::string_view datagram)
{
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());

    _gateway.onDatagram(addressOf(sender), datagram, Clock::now(),
                        static_cast<std::uint64_t>(seconds.count()));
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
        if (!error) {
            dispatch(sender, std::string_view(buffer.data(), size));
        }
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
        for (auto &message : _bridge.step()) {
            _outgoingMessages.handleEnqueue(std::move(message));
        }
        accumulator -= timestep;
    }
    _tickCount.store(_bridge.tick());
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
        addPlayer(message);
    }
    if (PacketReader::hasRecord(message.payload, MessageType::INPUT)) {
        applyInputs(message);
    }
    /*
     * to do:
     * apply the other records of message.payload to the simulation state.
     */
}

void Server::addPlayer(const IncomingMessage &message)
{
    const auto slot = _players.join(message.clientId, message.slot);

    if (!slot) {
        return;
    }
    for (auto &world : _bridge.addClient(message.clientId, *slot)) {
        _outgoingMessages.handleEnqueue(std::move(world));
    }
}

void Server::applyInputs(const IncomingMessage &message)
{
    std::string_view records = PacketReader::recordsOf(message.payload);

    while (const auto record = PacketReader::readRecord(records)) {
        const auto decoded =
            protocol::MessageCodec::decodeBody(record->type, record->body);
        const auto *input =
            decoded ? std::get_if<protocol::Input>(&*decoded) : nullptr;

        if (input != nullptr) {
            _bridge.onInput(message.clientId, *input);
        }
    }
}

void Server::removePlayer(std::uint32_t clientId, DisconnectReason reason)
{
    _bridge.removeClient(clientId);
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
              << _bridge.rules().tickRate << " Hz\n";

    runSimulationLoop(stopToken, timestep);

    std::cout << "[server] Simulation thread stopped after "
              << _tickCount.load() << " ticks\n";
}

} /* namespace rtype::server */
