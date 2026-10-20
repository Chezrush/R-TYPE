/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** Server
*/

#include <iostream>
#include "Server.hpp"

namespace rtype::server
{

Server::Server(ServerConfig config) noexcept
    : config_(config)
{
}

Server::~Server()
{
    shutdown();
}

bool Server::start()
{
    bool expected = false;

    if (!running_.compare_exchange_strong(expected, true)) {
        return false;
    }

    tickCount_.store(0);

    networkThread_ = std::jthread(
        [this](std::stop_token stopToken) {
            runNetwork(stopToken);
        });

    simulationThread_ = std::jthread(
        [this](std::stop_token stopToken) {
            runSimulation(stopToken);
        });

    return true;
}

void Server::shutdown() noexcept
{
    if (!running_.exchange(false)) {
        return;
    }

    if (networkThread_.joinable()) {
        networkThread_.request_stop();
        networkThread_.join();
    }

    if (simulationThread_.joinable()) {
        simulationThread_.request_stop();
        simulationThread_.join();
    }
}

bool Server::isRunning() const noexcept
{
    return running_.load();
}

std::uint64_t Server::tickCount() const noexcept
{
    return tickCount_.load();
}

std::chrono::nanoseconds Server::simulationInterval() const noexcept
{
    const auto hz = config_.simulationHz > 0 ? config_.simulationHz : 60;
    double castedHz = static_cast<double>(hz);

    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(1.0 / castedHz));
}

void Server::runNetwork(std::stop_token stopToken)
{
    std::cout << "[server] Network thread started\n";

    while (!stopToken.stop_requested()) {
        /*
         * to do:
         * make the udp socket with asio and fix the protocol.
         */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[server] Network thread stopped\n";
}

void Server::runSimulation(std::stop_token stopToken)
{
    using Clock = std::chrono::steady_clock;

    const auto timestep = simulationInterval();
    auto previousTime = Clock::now();
    auto accumulator = std::chrono::nanoseconds::zero();

    std::cout << "[server] Simulation thread started at "
              << config_.simulationHz << " Hz\n";

    while (!stopToken.stop_requested()) {
        const auto currentTime = Clock::now();
        auto frameTime = currentTime - previousTime;
        previousTime = currentTime;

        if (frameTime > config_.maxFrameTime) {
            frameTime = config_.maxFrameTime;
        }

        accumulator += frameTime;

        while (accumulator >= timestep && !stopToken.stop_requested()) {
            /*
             * to do:
             * checks and updates here
             */
            tickCount_.fetch_add(1);
            accumulator -= timestep;
        }

        std::this_thread::sleep_until(previousTime + timestep);
    }

    std::cout << "[server] Simulation thread stopped after "
              << tickCount_.load() << " ticks\n";
}

} /* namespace rtype::server */