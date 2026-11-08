/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ServerTests.cpp
*/

#include <chrono>
#include <string>
#include <thread>

#include "PacketBuilder.hpp"
#include "Server.hpp"
#include "TestSupport.hpp"

using rtype::protocol::MessageHeader;
using rtype::protocol::PacketBuilder;
using rtype::server::IncomingMessage;
using rtype::server::Server;
using rtype::server::ServerConfig;

void testServerStartsAndStops()
{
    Server server;

    CHECK(server.isRunning() == false);
    CHECK(server.start() == true);
    CHECK(server.isRunning() == true);

    server.shutdown();

    CHECK(server.isRunning() == false);
}

void testSimulationRun()
{
    ServerConfig config;
    config.simulationHz = 100;

    Server server(config);
    const auto begin = std::chrono::steady_clock::now();

    CHECK(server.start() == true);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    const auto ticks = server.tickCount();
    const std::chrono::duration<double> elapsed =
        std::chrono::steady_clock::now() - begin;
    const double expected = elapsed.count() * config.simulationHz;

    /*
     * The simulation catches up after a stall, so the number of ticks
     * follows the time really elapsed, not the requested sleep. A loaded
     * machine can sleep much longer than 250 ms. Allow scheduling jitter,
     * but ensure the simulation has clearly run independently from any
     * network client.
     */
    CHECK(ticks >= expected * 0.5);
    CHECK(ticks <= expected * 1.2 + 2);
    server.shutdown();
}

void testShutdown()
{
    Server server;

    CHECK(server.start() == true);

    server.shutdown();
    server.shutdown();

    CHECK(server.isRunning() == false);
}

void testRejectedPacketsAreCounted()
{
    ServerConfig config;
    std::string valid;

    config.port = 0;
    Server server(config);
    PacketBuilder::writeHeader(valid, MessageHeader{});
    PacketBuilder::appendHello(valid, 0, "Bob");

    CHECK(!server.receivePacket(IncomingMessage{1, std::string(3, 0), {}}));
    CHECK(server.receivePacket(IncomingMessage{1, valid, {}}));
    CHECK(server.rejectedPackets() == 1);
}

int main()
{
    testServerStartsAndStops();
    testSimulationRun();
    testShutdown();
    testRejectedPacketsAreCounted();

    return TEST_RESULT();
}
