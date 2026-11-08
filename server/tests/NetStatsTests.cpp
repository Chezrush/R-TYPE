/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetStatsTests.cpp
*/

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ClientHandshake.hpp"
#include "Gateway.hpp"
#include "NetStats.hpp"
#include "TestSupport.hpp"

using namespace rtype::server;
using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace {

struct Rig {
    std::vector<std::pair<std::string, std::string>> sent;
    Gateway gateway;
    ClientHandshake handshake;
    std::unique_ptr<Connection> connection;
    std::string address = "1.1.1.1|1000";

    explicit Rig(TimePoint now)
        : gateway(
              GatewayConfig{}, SipKey{},
              [this](const std::string &to, const std::string &data) {
                  sent.emplace_back(to, data);
              },
              [](const IncomingMessage &) { return true; },
              [](std::uint32_t, DisconnectReason) {}),
          handshake("Bob", now)
    {}

    void pump()
    {
        const auto batch = std::move(sent);

        sent.clear();
        for (const auto &item : batch) {
            handshake.onDatagram(item.second);
        }
        if (!connection &&
            handshake.state() == ClientHandshake::State::CONNECTED) {
            connection = std::make_unique<Connection>(handshake.session());
        }
    }

    bool connect(TimePoint &now)
    {
        for (int step = 0; step < 20 && !connection; ++step, now += 600ms) {
            if (const auto hello = handshake.poll(now)) {
                gateway.onDatagram(address, *hello, now, 1000);
            }
            pump();
        }
        return connection != nullptr;
    }

    void sendInput(TimePoint now)
    {
        const std::vector<Message> input = {Input{1, {0, 1}}};

        gateway.onDatagram(address, connection->buildDatagram(now, input), now,
                           1000);
    }
};

void testCountersAfterHandshakeAndInputs()
{
    TimePoint now = Clock::now();
    Rig rig(now);

    CHECK(rig.connect(now));
    const GatewayStats before = rig.gateway.stats();

    CHECK(before.clients.size() == 1);
    CHECK(before.totals.datagramsIn >= 2);
    CHECK(before.totals.datagramsOut >= 2);
    CHECK(before.totals.bytesIn > 0);
    CHECK(before.totals.bytesOut > 0);
    for (int i = 0; i < 3; ++i) {
        rig.sendInput(now + 10ms * i);
    }
    const GatewayStats after = rig.gateway.stats();

    CHECK(after.totals.datagramsIn == before.totals.datagramsIn + 3);
    CHECK(after.clients[0].traffic.datagramsIn ==
          before.clients[0].traffic.datagramsIn + 3);
    CHECK(after.totals.dropped == 0);
}

void testInvalidDatagramsAreCounted()
{
    TimePoint now = Clock::now();
    Rig rig(now);

    CHECK(rig.connect(now));
    const NetCounters before = rig.gateway.stats().totals;
    std::string broken = rig.connection->buildDatagram(now);

    broken.replace(0, 4, "xxxx");
    rig.gateway.onDatagram("6.6.6.6|1", "junk", now, 1000);
    rig.gateway.onDatagram(rig.address, broken, now, 1000);
    const GatewayStats after = rig.gateway.stats();

    CHECK(after.totals.dropped == before.dropped + 2);
    CHECK(after.totals.datagramsIn == before.datagramsIn + 2);
    CHECK(after.clients[0].traffic.dropped == 1);
}

void testRatesUseTheGivenClock()
{
    NetRateMeter meter;
    NetCounters totals;
    const NetRateMeter::TimePoint start = std::chrono::steady_clock::now();

    CHECK(meter.sample(totals, start).bytesInPerSecond == 0.0);
    totals.recordIn(1000);
    totals.recordOut(4000);
    const NetRates rates = meter.sample(totals, start + 2s);

    CHECK(rates.bytesInPerSecond == 500.0);
    CHECK(rates.bytesOutPerSecond == 2000.0);
    CHECK(meter.sample(totals, start + 4s).bytesInPerSecond == 0.0);
    CHECK(meter.sample(totals, start + 4s).bytesOutPerSecond == 0.0);
}

void testLineFormat()
{
    const NetRates rates{1.4 * 1024.0, 4.5 * 1024.0};

    CHECK(formatNetLine(2, rates, 0, 3ms) ==
          "[net] clients=2 in=1.4 KB/s out=4.5 KB/s dropped=0 rtt=3ms");
}

} // namespace

int main()
{
    testCountersAfterHandshakeAndInputs();
    testInvalidDatagramsAreCounted();
    testRatesUseTheGivenClock();
    testLineFormat();
    return TEST_RESULT();
}
