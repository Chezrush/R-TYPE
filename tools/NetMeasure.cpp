/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetMeasure.cpp
*/

#include <algorithm>
#include <array>
#include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "ByteReader.hpp"
#include "ClientHandshake.hpp"
#include "Connection.hpp"
#include "MessageCodec.hpp"
#include "PacketReader.hpp"
#include "Server.hpp"

using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace {

using Udp = asio::ip::udp;
using Peers = std::vector<std::unique_ptr<class LoopbackPeer>>;

constexpr std::size_t IP_UDP_OVERHEAD = 28;
constexpr double TARGET_BYTES_PER_SECOND = 5000.0;
constexpr std::size_t TOTAL_MESSAGES = 200;

std::size_t snapshotDatagramSize(std::size_t entities)
{
    Snapshot snapshot;

    snapshot.entities.resize(entities);
    return PACKET_HEADER_SIZE + MessageCodec::recordSize(snapshot);
}

std::string verdict(double bytesPerSecond)
{
    return bytesPerSecond <= TARGET_BYTES_PER_SECOND ? "yes" : "no";
}

void printSizeRow(std::size_t entities)
{
    const std::size_t size = snapshotDatagramSize(entities);
    const std::size_t onWire = size + IP_UDP_OVERHEAD;
    const double at20 = static_cast<double>(onWire) * 20.0;
    const double at60 = static_cast<double>(onWire) * 60.0;

    std::cout << "| " << entities << " | " << size << " | " << onWire << " | "
              << at20 << " | " << at60 << " | " << verdict(at20) << " | "
              << verdict(at60) << " |\n";
}

void measureSnapshots()
{
    std::cout << "\n### Snapshot size and bandwidth per client\n\n"
              << "| Entities | Datagram (B) | With IP+UDP (B) | 20 Hz "
              << "(B/s) | 60 Hz (B/s) | 20 Hz <= 5 KB/s | 60 Hz <= 5 KB/s "
              << "|\n|---|---|---|---|---|---|---|\n";
    for (const std::size_t entities : {1U, 10U, 30U, 60U}) {
        printSizeRow(entities);
    }
}

struct Scenario {
    int lossPercent = 0;
    int duplicatePercent = 0;
    int latencyMs = 0;
};

class Link {
public:
    Link(const Scenario &scenario, std::uint32_t seed)
        : _random(seed),
          _scenario(scenario)
    {}

    void send(const std::string &packet, TimePoint now)
    {
        if (static_cast<int>(_random() % 100) < _scenario.lossPercent) {
            return;
        }
        const bool twice =
            static_cast<int>(_random() % 100) < _scenario.duplicatePercent;
        const TimePoint at =
            now + std::chrono::milliseconds(_scenario.latencyMs);

        for (int i = 0; i < (twice ? 2 : 1); ++i) {
            _flight.emplace_back(at, packet);
        }
    }

    std::vector<std::string> arrived(TimePoint now)
    {
        std::vector<std::string> out;

        while (!_flight.empty() && _flight.front().first <= now) {
            out.push_back(std::move(_flight.front().second));
            _flight.pop_front();
        }
        return out;
    }

private:
    std::mt19937 _random;
    Scenario _scenario;
    std::deque<std::pair<TimePoint, std::string>> _flight;
};

struct Outcome {
    std::size_t delivered = 0;
    double meanMs = 0;
    double maxMs = 0;
    std::size_t retransmissions = 0;
    std::size_t wireBytes = 0;
    double overheadPercent = 0;
};

double toMs(Duration duration)
{
    return std::chrono::duration<double, std::milli>(duration).count();
}

class ReliableRun {
public:
    explicit ReliableRun(const Scenario &scenario)
        : _server(1),
          _client(1),
          _toClient(scenario, 7),
          _toServer(scenario, 11),
          _start(Clock::now())
    {}

    Outcome run()
    {
        TimePoint now = _start;

        for (int step = 0; _delivered.size() < TOTAL_MESSAGES && step < 24000;
             ++step, now += 5ms) {
            queueDue(step, now);
            transmit(step, now);
            receive(now);
        }
        return summarize();
    }

private:
    void queueDue(int step, TimePoint now)
    {
        if (step % 10 != 0 || _queuedAt.size() >= TOTAL_MESSAGES) {
            return;
        }
        const auto id = static_cast<std::uint32_t>(_queuedAt.size());

        _server.queueReliable(Score{0, id});
        _queuedAt.push_back(now);
    }

    void countRecords(const std::string &packet)
    {
        std::string_view records = PacketReader::recordsOf(packet);

        while (const auto record = PacketReader::readRecord(records)) {
            if ((record->type & RELIABLE_BIT) == 0) {
                continue;
            }
            ByteReader reader(record->body);

            if (!_seen.insert(reader.readU16()).second) {
                ++_retransmissions;
            }
        }
    }

    void transmit(int step, TimePoint now)
    {
        if (step % 10 == 0 || _server.hasDueReliable(now)) {
            const std::string packet = _server.buildDatagram(now);

            _wireBytes += packet.size();
            countRecords(packet);
            _toClient.send(packet, now);
        }
        if (step % 10 == 0) {
            const std::string packet = _client.buildDatagram(now);

            _wireBytes += packet.size();
            _toServer.send(packet, now);
        }
    }

    void receive(TimePoint now)
    {
        for (const auto &packet : _toClient.arrived(now)) {
            for (const Message &m : _client.receive(packet, now).reliable) {
                const auto id = std::get<Score>(m).score;

                _delivered.push_back(
                    std::chrono::duration_cast<Duration>(now - _queuedAt[id]));
            }
        }
        for (const auto &packet : _toServer.arrived(now)) {
            static_cast<void>(_server.receive(packet, now));
        }
    }

    Outcome summarize() const
    {
        Outcome out;
        double sum = 0;

        out.delivered = _delivered.size();
        for (const Duration latency : _delivered) {
            sum += toMs(latency);
            out.maxMs = std::max(out.maxMs, toMs(latency));
        }
        out.meanMs =
            out.delivered == 0 ? 0.0 : sum / static_cast<double>(out.delivered);
        out.retransmissions = _retransmissions;
        out.wireBytes = _wireBytes;
        const double payload = static_cast<double>(TOTAL_MESSAGES) * 5.0;
        out.overheadPercent =
            (static_cast<double>(_wireBytes) - payload) * 100.0 / payload;
        return out;
    }

    Connection _server;
    Connection _client;
    Link _toClient;
    Link _toServer;
    TimePoint _start;
    std::vector<TimePoint> _queuedAt;
    std::vector<Duration> _delivered;
    std::set<std::uint16_t> _seen;
    std::size_t _retransmissions = 0;
    std::size_t _wireBytes = 0;
};

void printOutcome(const Scenario &scenario, const Outcome &out)
{
    std::cout << std::fixed << std::setprecision(1) << "| "
              << scenario.lossPercent << " | " << scenario.duplicatePercent
              << " | " << scenario.latencyMs << " | " << out.delivered << "/"
              << TOTAL_MESSAGES << " | " << out.meanMs << " | " << out.maxMs
              << " | " << out.retransmissions << " | " << out.wireBytes << " | "
              << out.overheadPercent << " |\n";
}

void measureReliable()
{
    std::cout << "\n### Reliable delivery of 200 messages (in order)\n\n"
              << "| Loss % | Dup % | One-way ms | Delivered | Mean ms | "
              << "Max ms | Retransmitted records | Wire bytes | Overhead "
              << "% |\n|---|---|---|---|---|---|---|---|---|\n";
    for (const int latency : {0, 75, 150}) {
        for (const int loss : {0, 2, 10, 30}) {
            const Scenario scenario{loss, loss == 0 ? 0 : 2, latency};
            ReliableRun run(scenario);

            printOutcome(scenario, run.run());
        }
    }
}

struct Counters {
    std::size_t bytesIn = 0;
    std::size_t bytesOut = 0;
    std::size_t datagramsIn = 0;
    std::size_t maxEntities = 0;
};

class LoopbackPeer {
public:
    LoopbackPeer(std::uint16_t port, const std::string &name)
        : _socket(_context, Udp::endpoint(Udp::v4(), 0)),
          _server(asio::ip::make_address("127.0.0.1"), port),
          _handshake(name, Clock::now())
    {
        _socket.non_blocking(true);
    }

    void pump(bool sendInput)
    {
        if (!_connection) {
            if (const auto hello = _handshake.poll(Clock::now())) {
                send(*hello);
            }
        }
        drain();
        if (sendInput && _connection) {
            sendInputSample();
        }
    }

    [[nodiscard]] bool connected() const
    {
        return _connection != nullptr;
    }

    [[nodiscard]] const Counters &counters() const
    {
        return _counters;
    }

    void resetCounters()
    {
        _counters = Counters{};
    }

private:
    void drain()
    {
        std::array<char, 2048> buffer{};
        Udp::endpoint from;
        std::error_code error;

        for (;;) {
            const auto size =
                _socket.receive_from(asio::buffer(buffer), from, 0, error);

            if (error) {
                return;
            }
            handle(std::string(buffer.data(), size));
        }
    }

    void send(const std::string &datagram)
    {
        std::error_code error;

        _counters.bytesOut += datagram.size();
        _socket.send_to(asio::buffer(datagram), _server, 0, error);
    }

    void sendInputSample()
    {
        Input input;

        input.firstId = _nextInput++;
        input.keys = {KEY_RIGHT | KEY_SHOOT};
        const std::vector<Message> messages = {input};

        send(_connection->buildDatagram(Clock::now(), messages));
    }

    bool establish(const std::string &datagram)
    {
        if (_connection) {
            return true;
        }
        _handshake.onDatagram(datagram);
        if (_handshake.state() != ClientHandshake::State::CONNECTED) {
            return false;
        }
        _connection = std::make_unique<Connection>(_handshake.session());
        return true;
    }

    void handle(const std::string &datagram)
    {
        if (!establish(datagram)) {
            return;
        }
        _counters.bytesIn += datagram.size();
        ++_counters.datagramsIn;
        const auto result = _connection->receive(datagram, Clock::now());

        for (const Message &message : result.unreliable) {
            if (const auto *snap = std::get_if<Snapshot>(&message)) {
                _counters.maxEntities =
                    std::max(_counters.maxEntities, snap->entities.size());
            }
        }
    }

    asio::io_context _context;
    Udp::socket _socket;
    Udp::endpoint _server;
    ClientHandshake _handshake;
    std::unique_ptr<Connection> _connection;
    Counters _counters;
    std::uint16_t _nextInput = 0;
};

bool connectAll(Peers &peers)
{
    const auto deadline = Clock::now() + 3s;

    while (Clock::now() < deadline) {
        bool all = true;

        for (auto &peer : peers) {
            peer->pump(false);
            all = all && peer->connected();
        }
        if (all) {
            return true;
        }
        std::this_thread::sleep_for(5ms);
    }
    return false;
}

void runFor(Peers &peers, std::chrono::milliseconds duration)
{
    const auto end = Clock::now() + duration;

    while (Clock::now() < end) {
        for (auto &peer : peers) {
            peer->pump(true);
        }
        std::this_thread::sleep_for(16ms);
    }
}

Counters collect(const Peers &peers)
{
    Counters sum;

    for (const auto &peer : peers) {
        sum.bytesIn += peer->counters().bytesIn;
        sum.bytesOut += peer->counters().bytesOut;
        sum.datagramsIn += peer->counters().datagramsIn;
        sum.maxEntities =
            std::max(sum.maxEntities, peer->counters().maxEntities);
    }
    return sum;
}

void printLoopback(std::size_t count, const Counters &sum, double seconds)
{
    const double perPeer = seconds * static_cast<double>(count);

    std::cout << std::fixed << std::setprecision(0) << "| " << count << " | "
              << static_cast<double>(sum.bytesIn) / perPeer << " | "
              << static_cast<double>(sum.bytesOut) / perPeer << " | "
              << static_cast<double>(sum.datagramsIn) / perPeer << " | "
              << sum.maxEntities << " |\n";
}

Peers makePeers(std::uint16_t port, std::size_t count)
{
    Peers peers;

    for (std::size_t i = 0; i < count; ++i) {
        peers.push_back(
            std::make_unique<LoopbackPeer>(port, "P" + std::to_string(i)));
    }
    return peers;
}

void loopbackRun(std::size_t count, int millis)
{
    rtype::server::ServerConfig config;
    Counters sum;

    config.port = 0;
    rtype::server::Server server(config);
    server.start();
    Peers peers = makePeers(server.port(), count);
    if (connectAll(peers)) {
        for (auto &peer : peers) {
            peer->resetCounters();
        }
        runFor(peers, std::chrono::milliseconds(millis));
        sum = collect(peers);
    }
    server.shutdown();
    printLoopback(count, sum, millis / 1000.0);
}

void measureLoopback()
{
    std::cout << "\n### Real loopback UDP (3 s, inputs every 16 ms)\n\n"
              << "| Peers | Server to client (B/s per peer) | Client to "
              << "server (B/s per peer) | Datagrams/s per peer | Max "
              << "entities in a snapshot |\n|---|---|---|---|---|\n";
    for (const std::size_t count : {1U, 2U, 3U, 4U}) {
        loopbackRun(count, 3000);
    }
}

} /* namespace */

int main()
{
    try {
        measureSnapshots();
        measureReliable();
        measureLoopback();
    } catch (const std::exception &error) {
        std::cerr << "measurement failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
