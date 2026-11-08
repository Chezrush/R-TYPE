/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetworkTests.cpp
*/

#include <asio.hpp>
#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "ClientHandshake.hpp"
#include "Connection.hpp"
#include "PacketReader.hpp"
#include "Server.hpp"
#include "TestSupport.hpp"

using namespace rtype::server;
using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace {

using Udp = asio::ip::udp;

ServerConfig testConfig(std::chrono::milliseconds timeout)
{
    ServerConfig config;

    config.port = 0;
    config.inactivityTimeout = timeout;
    return config;
}

bool waitFor(const std::function<bool()> &condition)
{
    const auto deadline = std::chrono::steady_clock::now() + 3s;

    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::sleep_for(5ms);
    }
    return true;
}

/**
 * @brief A real UDP client built on the protocol library only.
 */
class TestPeer {
public:
    explicit TestPeer(std::uint16_t port, const std::string &name)
        : _socket(_context, Udp::endpoint(Udp::v4(), 0)),
          _server(asio::ip::make_address("127.0.0.1"), port),
          _handshake(name, Clock::now())
    {
        _socket.non_blocking(true);
    }

    bool connect()
    {
        return waitFor([this] {
            pump();
            return _connection != nullptr;
        });
    }

    void pump()
    {
        if (!_connection) {
            if (const auto hello = _handshake.poll(Clock::now())) {
                send(*hello);
            }
        }
        for (const std::string &datagram : receiveAll()) {
            handle(datagram);
        }
    }

    void send(const std::string &datagram)
    {
        std::error_code error;

        _socket.send_to(asio::buffer(datagram), _server, 0, error);
    }

    void sendGoodbye()
    {
        const std::vector<Message> goodbye = {Goodbye{0}};

        send(_connection->buildDatagram(Clock::now(), goodbye));
    }

    [[nodiscard]] const ClientHandshake &handshake() const
    {
        return _handshake;
    }

    [[nodiscard]] const std::vector<Message> &reliable() const
    {
        return _reliable;
    }

    [[nodiscard]] bool gotGoodbye() const
    {
        return _gotGoodbye;
    }

private:
    std::vector<std::string> receiveAll()
    {
        std::vector<std::string> datagrams;
        std::array<char, 2048> buffer{};
        Udp::endpoint from;
        std::error_code error;

        for (;;) {
            const std::size_t size =
                _socket.receive_from(asio::buffer(buffer), from, 0, error);

            if (error) {
                return datagrams;
            }
            datagrams.emplace_back(buffer.data(), size);
        }
    }

    void handle(const std::string &datagram)
    {
        if (!_connection) {
            _handshake.onDatagram(datagram);
            if (_handshake.state() != ClientHandshake::State::CONNECTED) {
                return;
            }
            _connection = std::make_unique<Connection>(_handshake.session());
        }
        const auto result = _connection->receive(datagram, Clock::now());

        _reliable.insert(_reliable.end(), result.reliable.begin(),
                         result.reliable.end());
        for (const Message &message : result.unreliable) {
            _gotGoodbye =
                _gotGoodbye || std::holds_alternative<Goodbye>(message);
        }
        if (result.hadRecords) {
            send(_connection->buildDatagram(Clock::now()));
        }
    }

    asio::io_context _context;
    Udp::socket _socket;
    Udp::endpoint _server;
    ClientHandshake _handshake;
    std::unique_ptr<Connection> _connection;
    std::vector<Message> _reliable;
    bool _gotGoodbye = false;
};

bool receivedPlayerEvent(const TestPeer &peer, const Message &expected)
{
    for (const Message &message : peer.reliable()) {
        if (message == expected) {
            return true;
        }
    }
    return false;
}

void testHandshakeOverRealUdp()
{
    Server server(testConfig(5000ms));
    CHECK(server.start());
    TestPeer peer(server.port(), "Bob");

    CHECK(peer.connect());
    CHECK(peer.handshake().state() == ClientHandshake::State::CONNECTED);
    CHECK(peer.handshake().welcome() &&
          peer.handshake().welcome()->playerIndex == 0);
    CHECK(waitFor([&] { return server.playerCount() == 1; }));
    server.shutdown();
}

void testPlayersSeeEachOtherAndLeave()
{
    Server server(testConfig(5000ms));
    CHECK(server.start());
    TestPeer first(server.port(), "Bob");
    TestPeer second(server.port(), "Eve");

    CHECK(first.connect());
    CHECK(second.connect());
    CHECK(waitFor([&] {
        first.pump();
        second.pump();
        return receivedPlayerEvent(first, PlayerJoined{1, "Eve"}) &&
               receivedPlayerEvent(second, PlayerJoined{0, "Bob"});
    }));
    first.sendGoodbye();
    CHECK(waitFor([&] {
        second.pump();
        return server.playerCount() == 1 &&
               receivedPlayerEvent(second, PlayerLeft{0, 0});
    }));
    server.shutdown();
}

void testSilentClientTimesOutAndOthersAreTold()
{
    Server server(testConfig(400ms));
    CHECK(server.start());
    TestPeer silent(server.port(), "Bob");
    TestPeer alive(server.port(), "Eve");

    CHECK(silent.connect());
    CHECK(alive.connect());
    CHECK(waitFor([&] {
        alive.pump();
        return receivedPlayerEvent(alive, PlayerLeft{0, 1});
    }));
    CHECK(server.playerCount() == 1);
    server.shutdown();
}

void testGarbageNeverStopsTheServer()
{
    Server server(testConfig(5000ms));
    CHECK(server.start());
    asio::io_context context;
    Udp::socket socket(context, Udp::endpoint(Udp::v4(), 0));
    const Udp::endpoint target(asio::ip::make_address("127.0.0.1"),
                               server.port());

    for (int size = 0; size < 300; ++size) {
        socket.send_to(asio::buffer(std::string(size, '\x52')), target);
    }
    TestPeer peer(server.port(), "Bob");

    CHECK(peer.connect());
    CHECK(server.isRunning());
    server.shutdown();
}

void testServerSaysGoodbyeOnShutdown()
{
    Server server(testConfig(5000ms));
    CHECK(server.start());
    TestPeer peer(server.port(), "Bob");

    CHECK(peer.connect());
    server.shutdown();
    CHECK(waitFor([&] {
        peer.pump();
        return peer.gotGoodbye();
    }));
}

} // namespace

int main()
{
    testHandshakeOverRealUdp();
    testPlayersSeeEachOtherAndLeave();
    testSilentClientTimesOutAndOthersAreTold();
    testGarbageNeverStopsTheServer();
    testServerSaysGoodbyeOnShutdown();
    return TEST_RESULT();
}
