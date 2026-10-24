/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EventBus.cpp
*/

#include "Engine/Core/EventBus.hpp"

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

using engine::core::EventBus;

namespace {

struct Damage {
    int amount;
};

struct Death {
    int entity;
};

} // namespace

TEST_CASE("a listener receives the events of its type", "[core][bus]")
{
    EventBus bus;
    int total = 0;

    bus.subscribe<Damage>([&total](const Damage &e) { total += e.amount; });
    bus.publish(Damage{3});
    bus.publish(Damage{4});
    REQUIRE(total == 7);
}

TEST_CASE("a listener ignores the events of other types", "[core][bus]")
{
    EventBus bus;
    int calls = 0;

    bus.subscribe<Damage>([&calls](const Damage &) { ++calls; });
    bus.publish(Death{1});
    REQUIRE(calls == 0);
}

TEST_CASE("listeners are called in subscription order", "[core][bus]")
{
    EventBus bus;
    std::vector<int> order;

    bus.subscribe<Damage>([&order](const Damage &) { order.push_back(1); });
    bus.subscribe<Death>([&order](const Death &) { order.push_back(9); });
    bus.subscribe<Damage>([&order](const Damage &) { order.push_back(2); });
    bus.subscribe<Damage>([&order](const Damage &) { order.push_back(3); });
    bus.publish(Damage{0});
    REQUIRE(order == std::vector<int>{1, 2, 3});
}

TEST_CASE("an unsubscribed listener no longer receives events", "[core][bus]")
{
    EventBus bus;
    int first = 0;
    int second = 0;
    const auto id =
        bus.subscribe<Damage>([&first](const Damage &) { ++first; });

    bus.subscribe<Damage>([&second](const Damage &) { ++second; });
    REQUIRE(bus.unsubscribe(id));
    bus.publish(Damage{1});
    REQUIRE(first == 0);
    REQUIRE(second == 1);
    REQUIRE_FALSE(bus.unsubscribe(id));
}

TEST_CASE("publishing without listeners has no effect", "[core][bus]")
{
    EventBus bus;

    REQUIRE_NOTHROW(bus.publish(Damage{1}));
    REQUIRE(bus.size() == 0);
}

TEST_CASE("identifiers are never reused", "[core][bus]")
{
    EventBus bus;
    const auto first = bus.subscribe<Damage>([](const Damage &) {});

    REQUIRE(bus.unsubscribe(first));
    const auto second = bus.subscribe<Damage>([](const Damage &) {});
    REQUIRE(second != first);
    REQUIRE(bus.size() == 1);
}

TEST_CASE("an empty listener is rejected", "[core][bus]")
{
    EventBus bus;

    REQUIRE_THROWS_AS(
        bus.subscribe<Damage>(std::function<void(const Damage &)>()),
        std::invalid_argument);
}

TEST_CASE("events can carry non-trivial values", "[core][bus]")
{
    EventBus bus;
    std::string received;

    bus.subscribe<std::string>(
        [&received](const std::string &text) { received = text; });
    bus.publish(std::string("hit"));
    REQUIRE(received == "hit");
}

TEST_CASE("using the bus during a dispatch is rejected", "[core][bus]")
{
    EventBus bus;
    int rejected = 0;

    bus.subscribe<Damage>([&](const Damage &) {
        try {
            bus.publish(Death{1});
        } catch (const std::logic_error &) {
            ++rejected;
        }
        try {
            bus.subscribe<Death>([](const Death &) {});
        } catch (const std::logic_error &) {
            ++rejected;
        }
    });
    bus.publish(Damage{1});
    REQUIRE(rejected == 2);
}

TEST_CASE("a throwing listener leaves the bus usable", "[core][bus]")
{
    EventBus bus;
    int calls = 0;

    bus.subscribe<Damage>(
        [](const Damage &) { throw std::runtime_error("x"); });
    bus.subscribe<Damage>([&calls](const Damage &) { ++calls; });
    REQUIRE_THROWS_AS(bus.publish(Damage{1}), std::runtime_error);
    REQUIRE(calls == 0);
    REQUIRE_NOTHROW(bus.subscribe<Death>([](const Death &) {}));
}
