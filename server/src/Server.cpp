/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** Server
*/

#include "Server.hpp"
#include <iostream>
#include <utility>
#include "PacketChecker.hpp"

namespace rtype::server {

using Nanosecond = std::chrono::nanoseconds;

Server::Server(ServerConfig config)
    : _config(config),
      _incomingMessages(config.maxQueueCapacity),
      _outgoingMessages(config.maxQueueCapacity),
      _socket(_context, Udp::endpoint(Udp::v4(), config.port))
{}

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
    const double hz = _config.simulationHz > 0 ? _config.simulationHz : 60;
    const auto castedHz = static_cast<double>(hz);

    return std::chrono::duration_cast<Nanosecond>(
        std::chrono::duration<double>(1.0 / castedHz));
}

void Server::runNetwork(const std::stop_token &stopToken)
{
    std::cout << "[server] Network thread started\n";
    while (!stopToken.stop_requested()) {
        sendOutgoingMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << "[server] Network thread stopped\n";
}

void Server::sendOutgoingMessages()
{
    while (auto message = _outgoingMessages.handleDequeue()) {
        /*
         * to do:
         * serialize and send message->payload to message->clientId.
         */
        std::cout << "[server] Outgoing message for client "
                  << message->clientId << ": " << message->payload << "\n";
    }
}

void Server::updateSimulationTick(Nanosecond timestep, Nanosecond &accumulator,
                                  const std::stop_token &stopToken)
{
    while (accumulator >= timestep && !stopToken.stop_requested()) {
        _tickCount.fetch_add(1);
        accumulator -= timestep;
    }
}

void Server::readIncomingMessage()
{
    while (auto message = _incomingMessages.handleDequeue()) {
        /*
         * to do:
         * apply message->payload to the simulation state.
         */
    }
}

bool Server::receivePacket(IncomingMessage message)
{
    if (!PacketChecker::isValid(message.payload)) {
        _rejectedPackets.fetch_add(1);
        return false;
    }
    return _incomingMessages.handleEnqueue(std::move(message));
}

std::uint64_t Server::rejectedPackets() const noexcept
{
    return _rejectedPackets.load();
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
