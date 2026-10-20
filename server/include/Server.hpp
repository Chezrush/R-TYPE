/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Server.hpp
*/

#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

namespace rtype::server
{
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
};

class Server {
public:
    explicit Server(ServerConfig config = {}) noexcept;
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

private:
    void runNetwork(std::stop_token stopToken);
    void runSimulation(std::stop_token stopToken);

    [[nodiscard]] std::chrono::nanoseconds simulationInterval() const noexcept;

    ServerConfig config_;
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> tickCount_{0};

    std::jthread networkThread_;
    std::jthread simulationThread_;
};

} /* namespace rtype::server */


#endif /* !SERVER_HPP_ */
