/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Server.hpp
*/

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <algorithm>
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>

#include "ClientActivity.hpp"
#include "GameBridge.hpp"
#include "Gateway.hpp"
#include "NetStats.hpp"
#include "PlayerSessions.hpp"
#include "ServerMessage.hpp"

namespace rtype::server {

using Udp = asio::ip::udp;

struct ServerConfig {
    /**
     * @brief Simulation frequency, in ticks per second.
     */
    std::uint32_t simulationHz = 60;

    /**
     * @brief Maximum simulated time accepted in a single iteration.
     *
     * This prevents an excessive catch-up loop after a long pause.
     */
    std::chrono::milliseconds maxFrameTime{250};

    /**
     * @brief Maximum number of messages retained by each queue.
     */
    std::size_t maxQueueCapacity = 1024;

    /**
     * @brief UDP port of the server, 0 picks a free one.
     */
    std::uint16_t port = 4242;

    /**
     * @brief A client silent for longer than this is disconnected.
     */
    std::chrono::milliseconds inactivityTimeout{5000};

    /**
     * @brief Prints a network statistics line every statsInterval.
     */
    bool showStats = false;

    std::chrono::seconds statsInterval{5};
};

class Server {
public:
    explicit Server(ServerConfig config = {});
    ~Server();

    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    Server(Server &&) = delete;
    Server &operator=(Server &&) = delete;

    /**
     * @brief Starts the network and simulation threads.
     *
     * @return true if the server started, false if it was already running.
     */
    bool start();

    /**
     * @brief Requests shutdown and waits for every thread working to stop.
     */
    void shutdown() noexcept;

    /**
     * @brief Indicates whether the server is currently running.
     */
    [[nodiscard]] bool isRunning() const noexcept;

    /**
     * @brief Returns the number of simulation ticks performed since start.
     */
    [[nodiscard]] std::uint64_t tickCount() const noexcept;

    /**
     * @brief Called by runSimulation(), it only used to run the loop.
     */
    void runSimulationLoop(const std::stop_token &stopToken,
                           std::chrono::nanoseconds timestep);

    /**
     * @brief Queues the packet if it is valid, otherwise drops it.
     */
    bool receivePacket(IncomingMessage message);

    /**
     * @brief Returns the number of packets dropped by receivePacket().
     */
    [[nodiscard]] std::uint64_t rejectedPackets() const noexcept;

    /**
     * @brief Returns the UDP port the server listens on.
     */
    [[nodiscard]] std::uint16_t port() const;

    /**
     * @brief Returns the number of players in the simulation.
     */
    [[nodiscard]] std::size_t playerCount() const noexcept;

private:
    void runNetwork(const std::stop_token &stopToken);
    void receiveDatagrams();
    void dispatch(const Udp::endpoint &sender, std::string_view datagram);
    void sendDatagram(const std::string &address, const std::string &datagram);
    void runSimulation(const std::stop_token &stopToken);
    void updateSimulationTick(std::chrono::nanoseconds timestep,
                              std::chrono::nanoseconds &accumulator,
                              const std::stop_token &stopToken);
    void readIncomingMessage();
    void applyMessage(const IncomingMessage &message);
    void addPlayer(const IncomingMessage &message);
    void applyInputs(const IncomingMessage &message);
    void removePlayer(std::uint32_t clientId,
                      protocol::DisconnectReason reason);
    void sendOutgoingMessages();
    void disconnectInactiveClients();
    void printStats();
    void requestDisconnect(std::uint32_t clientId,
                           protocol::DisconnectReason reason);
    void handleGoodbye(std::uint32_t clientId);

    [[nodiscard]] std::chrono::nanoseconds simulationInterval() const noexcept;

    ServerConfig _config;
    std::atomic<bool> _running{false};
    std::atomic<std::uint64_t> _tickCount{0};
    std::atomic<std::uint64_t> _rejectedPackets{0};
    std::atomic<std::size_t> _playerCount{0};
    NetRateMeter _rateMeter;
    Clock::time_point _lastStatsLine = Clock::now();

    ClientActivity _activity;
    PlayerSessions _players;
    GameBridge _bridge;

    std::jthread _networkThread;
    std::jthread _simulationThread;

    IncomingQueue _incomingMessages;
    OutgoingQueue _outgoingMessages;

    asio::io_context _context;
    Udp::socket _socket;
    Gateway _gateway;
};

} /* namespace rtype::server */

#endif /* !SERVER_HPP_ */
