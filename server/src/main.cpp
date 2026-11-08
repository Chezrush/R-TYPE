/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** main.cpp
*/

#include <atomic>
#include <charconv>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>

#include "LocalAddress.hpp"
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

bool parsePort(const char *text, std::uint16_t &port)
{
    const std::string_view view(text);
    const char *first = std::to_address(view.begin());
    const char *last = std::to_address(view.end());
    unsigned int value = 0;
    const auto result = std::from_chars(first, last, value);

    if (result.ec != std::errc{} || result.ptr != last) {
        return false;
    }
    if (value < 1 || value > 65535) {
        return false;
    }
    port = static_cast<std::uint16_t>(value);
    return true;
}

void printAddresses(std::uint16_t port)
{
    const auto addresses = rtype::server::localAddresses();

    if (addresses.empty()) {
        std::cout << "No network address found, only this computer can join.\n";
        return;
    }
    std::cout << "Other computers on the network can join with:\n";
    for (const std::string &address : addresses) {
        std::cout << "  r-type_client " << address << " " << port << "\n";
    }
}

int run(const rtype::server::ServerConfig &config)
{
    rtype::server::Server server(config);

    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    std::cout << "r-type_server started on UDP port " << server.port()
              << ". Press Ctrl+C to stop.\n";
    printAddresses(server.port());
    waitForShutdown();
    std::cout << "Stopping r-type_server...\n";
    server.shutdown();
    std::cout << "r-type_server stopped cleanly.\n";
    return 0;
}

int main(int argc, char *const *argv)
{
    rtype::server::ServerConfig config;

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);
    if (argc > 1 && !parsePort(argv[1], config.port)) {
        std::cerr << "Usage: r-type_server [PORT]\n";
        return 1;
    }
    return run(config);
}
