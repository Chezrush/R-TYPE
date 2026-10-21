/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ServerTests.cpp
*/

#include <chrono>
#include <iostream>
#include <thread>

#include "Server.hpp"
#include "TestSupport.hpp"

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

    CHECK(server.start() == true);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    const auto ticks = server.tickCount();

    /*
     * Allow scheduling jitter, but ensure the simulation has clearly run
     * independently from any network client.
     */
    CHECK(ticks >= 15);
    CHECK(ticks <= 35);

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

int main()
{
    testServerStartsAndStops();
    testSimulationRun();
    testShutdown();

    return TEST_RESULT();
}
