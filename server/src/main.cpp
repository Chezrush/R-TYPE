/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** main.cpp
*/

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "Server.hpp"

std::atomic<bool> &stopRequestedFlag()
{
    static std::atomic<bool> flag{false};
    return flag;
}

void handleSignal(int)
{
    stopRequestedFlag().store(true);
}

void waitForShutdown()
{
    while (!stopRequestedFlag().load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main(int argc, char **argv)
{
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    rtype::server::ServerConfig config;

    if (argc > 1) {
        const int port = std::atoi(argv[1]);

        if (port < 1 || port > 65535) {
            std::cerr << "Usage: r-type_server [PORT]\n";
            return 1;
        }
        config.port = static_cast<std::uint16_t>(port);
    }
    rtype::server::Server server(config);

    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    std::cout << "r-type_server started. Press Ctrl+C to stop.\n";
    waitForShutdown();
    std::cout << "Stopping r-type_server...\n";
    server.shutdown();
    std::cout << "r-type_server stopped cleanly.\n";
    return 0;
}
