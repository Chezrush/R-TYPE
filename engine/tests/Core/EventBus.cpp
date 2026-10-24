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

TEST_CASE("an event published during a dispatch is queued", "[core][bus]")
{
    EventBus bus;
    std::vector<std::string> log;

    bus.subscribe<Damage>([&](const Damage &e) {
        log.push_back("damage" + std::to_string(e.amount));
        if (e.amount == 1) {
            bus.publish(Death{7});
            bus.publish(Damage{2});
        }
        log.push_back("end" + std::to_string(e.amount));
    });
    bus.subscribe<Death>([&](const Death &) { log.push_back("death"); });
    bus.publish(Damage{1});
    REQUIRE(log == std::vector<std::string>{"damage1", "end1", "death",
                                            "damage2", "end2"});
}

TEST_CASE("queued events keep the publication order", "[core][bus]")
{
    EventBus bus;
    std::vector<int> seen;

    bus.subscribe<Damage>([&](const Damage &e) {
        seen.push_back(e.amount);
        if (e.amount == 0) {
            bus.publish(Damage{1});
            bus.publish(Damage{2});
        }
        if (e.amount == 1)
            bus.publish(Damage{3});
    });
    bus.publish(Damage{0});
    REQUIRE(seen == std::vector<int>{0, 1, 2, 3});
}

TEST_CASE("a listener unsubscribed during a dispatch is not called again",
          "[core][bus]")
{
    EventBus bus;
    int second = 0;
    EventBus::ListenerId secondId = 0;

    bus.subscribe<Damage>([&](const Damage &) { bus.unsubscribe(secondId); });
    secondId = bus.subscribe<Damage>([&second](const Damage &) { ++second; });
    bus.publish(Damage{1});
    bus.publish(Damage{1});
    REQUIRE(second == 0);
    REQUIRE(bus.size() == 1);
}

TEST_CASE("a listener can unsubscribe itself", "[core][bus]")
{
    EventBus bus;
    int calls = 0;
    EventBus::ListenerId id = 0;

    id = bus.subscribe<Damage>([&](const Damage &) {
        ++calls;
        REQUIRE(bus.unsubscribe(id));
    });
    bus.publish(Damage{1});
    bus.publish(Damage{1});
    REQUIRE(calls == 1);
    REQUIRE(bus.size() == 0);
}

TEST_CASE("a listener subscribed during a dispatch misses the current event",
          "[core][bus]")
{
    EventBus bus;
    int late = 0;
    bool added = false;

    bus.subscribe<Damage>([&](const Damage &) {
        if (added)
            return;
        added = true;
        bus.subscribe<Damage>([&late](const Damage &) { ++late; });
    });
    bus.publish(Damage{1});
    REQUIRE(late == 0);
    bus.publish(Damage{1});
    REQUIRE(late == 1);
}

TEST_CASE("a listener subscribed during a dispatch gets queued events",
          "[core][bus]")
{
    EventBus bus;
    int late = 0;

    bus.subscribe<Damage>([&](const Damage &e) {
        if (e.amount != 1)
            return;
        bus.subscribe<Damage>([&late](const Damage &) { ++late; });
        bus.publish(Damage{2});
    });
    bus.publish(Damage{1});
    REQUIRE(late == 1);
}

TEST_CASE("a listener of a new type can subscribe during a dispatch",
          "[core][bus]")
{
    EventBus bus;
    int deaths = 0;

    bus.subscribe<Damage>([&](const Damage &) {
        bus.subscribe<Death>([&deaths](const Death &) { ++deaths; });
        bus.publish(Death{1});
    });
    bus.publish(Damage{1});
    REQUIRE(deaths == 1);
}

TEST_CASE("a throwing listener discards the queue and leaves the bus usable",
          "[core][bus]")
{
    EventBus bus;
    int deaths = 0;

    bus.subscribe<Damage>([&bus](const Damage &) {
        bus.publish(Death{1});
        throw std::runtime_error("x");
    });
    bus.subscribe<Death>([&deaths](const Death &) { ++deaths; });
    REQUIRE_THROWS_AS(bus.publish(Damage{1}), std::runtime_error);
    REQUIRE(deaths == 0);
    bus.publish(Death{1});
    REQUIRE(deaths == 1);
}
