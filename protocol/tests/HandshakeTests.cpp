/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HandshakeTests.cpp
*/

#include <string>

#include "ClientHandshake.hpp"
#include "Connection.hpp"
#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "ServerHandshake.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;
using namespace std::chrono_literals;

SipKey secret()
{
    SipKey key{};

    key.fill(0x42);
    return key;
}

std::string welcomeDatagram(std::uint32_t session, TimePoint now)
{
    Connection connection(session);
    const std::vector<Message> messages = {
        Welcome{1, 4, 60, 20, 1280, 720, 300, 0}};

    return connection.buildDatagram(now, messages);
}

ServerHandshake::Decision ask(ServerHandshake &server,
                              const std::string &packet, TimePoint now,
                              bool slot = true,
                              const char *address = "1.2.3.4:5000")
{
    return server.handle(address, packet, now, 1000, slot);
}

void testFullHandshake()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    ClientHandshake client("Bob", start);

    const auto hello = client.poll(start);
    CHECK(hello.has_value());
    const auto challenge = ask(server, *hello, start);

    CHECK(challenge.kind == ServerHandshake::Kind::REPLY);
    CHECK(challenge.reply.size() == 27);
    client.onDatagram(challenge.reply);
    const auto second = client.poll(start + 1ms);

    CHECK(second.has_value());
    const auto accepted = ask(server, *second, start + 1ms);

    CHECK(accepted.kind == ServerHandshake::Kind::ACCEPT);
    CHECK(accepted.hello.name == "Bob");
    const std::uint32_t session = ServerHandshake::newSession();

    client.onDatagram(welcomeDatagram(session, start));
    CHECK(client.state() == ClientHandshake::State::CONNECTED);
    CHECK(client.session() == session);
    CHECK(client.welcome() && client.welcome()->playerIndex == 1);
}

void testHelloIsRepeatedEvery500msAndGivesUpAfter10s()
{
    const TimePoint start = Clock::now();
    ClientHandshake client("Bob", start);

    CHECK(client.poll(start).has_value());
    CHECK(!client.poll(start + 499ms).has_value());
    CHECK(client.poll(start + 500ms).has_value());
    CHECK(client.poll(start + 9500ms).has_value());
    CHECK(!client.poll(start + 10s).has_value());
    CHECK(client.state() == ClientHandshake::State::FAILED);
}

void testLostChallengeIsRepairedByTheRetry()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    ClientHandshake client("Bob", start);

    static_cast<void>(client.poll(start));
    const auto retry = client.poll(start + 500ms);

    CHECK(retry.has_value());
    CHECK(ask(server, *retry, start + 500ms).kind ==
          ServerHandshake::Kind::REPLY);
}

void testCookieOfAnotherAddressIsRefused()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    ClientHandshake client("Bob", start);
    const auto challenge = ask(server, *client.poll(start), start);

    client.onDatagram(challenge.reply);
    const auto second = *client.poll(start + 1ms);
    const auto spoofed = ask(server, second, start + 1ms, true, "6.6.6.6:1");

    CHECK(spoofed.kind == ServerHandshake::Kind::REPLY);
    CHECK(spoofed.reply.size() == 27);
}

void testServerFullAndInvalidName()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    const std::uint64_t cookie =
        CookieFactory(secret()).make("1.2.3.4:5000", 1000);
    std::string full;
    std::string noName;

    PacketBuilder::writeHeader(full, MessageHeader{});
    MessageCodec::appendMessage(full, Hello{cookie, "Bob"});
    PacketBuilder::writeHeader(noName, MessageHeader{});
    MessageCodec::appendMessage(noName, Hello{cookie, ""});
    const auto fullAnswer = ask(server, full, start, false);
    const auto nameAnswer = ask(server, noName, start + 1s);
    ClientHandshake client("Bob", start);

    CHECK(fullAnswer.kind == ServerHandshake::Kind::REPLY);
    CHECK(fullAnswer.reply.size() == 20);
    client.onDatagram(fullAnswer.reply);
    CHECK(client.state() == ClientHandshake::State::REJECTED);
    CHECK(client.rejectReason() ==
          static_cast<std::uint8_t>(RejectReason::SERVER_FULL));
    CHECK(nameAnswer.kind == ServerHandshake::Kind::REPLY);
}

void testRepliesAreNeverLargerThanTheRequest()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    std::string hello;

    PacketBuilder::writeHeader(hello, MessageHeader{});
    MessageCodec::appendMessage(hello, Hello{0, ""});
    const auto answer = ask(server, hello, start);

    CHECK(hello.size() == 28);
    CHECK(answer.reply.size() <= hello.size());
}

void testVersionMismatchGetsOneReject()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    std::string hello;

    PacketBuilder::writeHeader(hello, MessageHeader{});
    MessageCodec::appendMessage(hello, Hello{0, "Bob"});
    hello[2] = 9;
    const auto first = ask(server, hello, start);
    const auto second = ask(server, hello, start + 100ms);
    const auto later = ask(server, hello, start + 2s);

    CHECK(first.kind == ServerHandshake::Kind::REPLY);
    CHECK(second.kind == ServerHandshake::Kind::DROP);
    CHECK(later.kind == ServerHandshake::Kind::REPLY);
}

void testClientReadsAServerVersion()
{
    const TimePoint start = Clock::now();
    ClientHandshake client("Bob", start);
    std::string reject;

    MessageHeader header;

    header.version = 7;
    PacketBuilder::writeHeader(reject, header);
    client.onDatagram(reject);
    CHECK(client.state() == ClientHandshake::State::REJECTED);
    CHECK(client.serverVersion() == 7);
    CHECK(client.rejectReason() ==
          static_cast<std::uint8_t>(RejectReason::VERSION));
}

void testHelloFloodIsRateLimited()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    std::string hello;
    int answered = 0;

    PacketBuilder::writeHeader(hello, MessageHeader{});
    MessageCodec::appendMessage(hello, Hello{0, "Bob"});
    for (int i = 0; i < 100; ++i) {
        answered +=
            ask(server, hello, start).kind != ServerHandshake::Kind::DROP;
    }
    CHECK(answered == 5);
}

void testOnlyASingleHelloWithNoSessionIsAccepted()
{
    ServerHandshake server(secret());
    const TimePoint start = Clock::now();
    std::string twoRecords;
    std::string withSession;
    MessageHeader header;

    PacketBuilder::writeHeader(twoRecords, header);
    MessageCodec::appendMessage(twoRecords, Hello{0, "Bob"});
    MessageCodec::appendMessage(twoRecords, Goodbye{0});
    header.session = 5;
    PacketBuilder::writeHeader(withSession, header);
    MessageCodec::appendMessage(withSession, Hello{0, "Bob"});
    CHECK(ask(server, twoRecords, start).kind == ServerHandshake::Kind::DROP);
    CHECK(ask(server, withSession, start).kind ==
          ServerHandshake::Kind::DROP);
    CHECK(ask(server, "junk", start).kind == ServerHandshake::Kind::DROP);
}

void testSessionsAreNeverZero()
{
    for (int i = 0; i < 1000; ++i) {
        CHECK(ServerHandshake::newSession() != 0);
    }
}

int main()
{
    testFullHandshake();
    testHelloIsRepeatedEvery500msAndGivesUpAfter10s();
    testLostChallengeIsRepairedByTheRetry();
    testCookieOfAnotherAddressIsRefused();
    testServerFullAndInvalidName();
    testRepliesAreNeverLargerThanTheRequest();
    testVersionMismatchGetsOneReject();
    testClientReadsAServerVersion();
    testHelloFloodIsRateLimited();
    testOnlyASingleHelloWithNoSessionIsAccepted();
    testSessionsAreNeverZero();
    return TEST_RESULT();
}
