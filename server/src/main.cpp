/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** main.cpp
*/

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include "Server.hpp"

namespace {

std::atomic<bool> g_stopRequested{false};

void handleSignal(int)
{
    g_stopRequested.store(true);
}

} /* namespace */

int main()
{
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    rtype::server::Server server;

    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    std::cout << "r-type_server started. Press Ctrl+C to stop.\n";

    while (!g_stopRequested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "Stopping r-type_server...\n";
    server.shutdown();
    std::cout << "r-type_server stopped cleanly.\n";

    return 0;
}