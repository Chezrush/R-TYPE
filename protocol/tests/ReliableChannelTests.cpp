/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReliableChannelTests.cpp
*/

#include <random>
#include <string>
#include <vector>

#include "Protocol.hpp"
#include "ReceiveWindow.hpp"
#include "ReliableReceiver.hpp"
#include "ReliableSender.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;
using namespace std::chrono_literals;

ReliableRecord record(std::uint16_t relId)
{
    return ReliableRecord{relId, 0x81, std::to_string(relId)};
}

void testInOrderDelivery()
{
    ReliableReceiver receiver;
    std::vector<ReliableRecord> out;

    CHECK(receiver.accept(record(0), out) == ReliableVerdict::DELIVERED);
    CHECK(receiver.accept(record(1), out) == ReliableVerdict::DELIVERED);
    CHECK(out.size() == 2);
    CHECK(receiver.nextExpected() == 2);
}

void testOutOfOrderIsBufferedThenReleased()
{
    ReliableReceiver receiver;
    std::vector<ReliableRecord> out;

    CHECK(receiver.accept(record(2), out) == ReliableVerdict::BUFFERED);
    CHECK(receiver.accept(record(1), out) == ReliableVerdict::BUFFERED);
    CHECK(out.empty());
    CHECK(receiver.buffered() == 2);
    CHECK(receiver.accept(record(0), out) == ReliableVerdict::DELIVERED);
    CHECK(out.size() == 3);
    CHECK(out[0].relId == 0 && out[1].relId == 1 && out[2].relId == 2);
    CHECK(receiver.buffered() == 0);
}

void testDuplicatesAreIgnored()
{
    ReliableReceiver receiver;
    std::vector<ReliableRecord> out;

    receiver.accept(record(0), out);
    CHECK(receiver.accept(record(0), out) == ReliableVerdict::DUPLICATE);
    receiver.accept(record(5), out);
    CHECK(receiver.accept(record(5), out) == ReliableVerdict::DUPLICATE);
    CHECK(out.size() == 1);
}

void testWindowLimit()
{
    ReliableReceiver receiver;
    std::vector<ReliableRecord> out;

    CHECK(receiver.inWindow(127));
    CHECK(!receiver.inWindow(128));
    CHECK(receiver.accept(record(128), out) == ReliableVerdict::OUT_OF_WINDOW);
    CHECK(receiver.buffered() == 0);
}

void testRelIdWrapAround()
{
    ReliableReceiver receiver;
    std::vector<ReliableRecord> out;

    for (std::uint32_t id = 0; id < 70000; ++id) {
        receiver.accept(record(static_cast<std::uint16_t>(id)), out);
    }
    CHECK(out.size() == 70000);
    CHECK(receiver.nextExpected() == 70000 - 65536);
}

void testSenderSendsEachRecordOnceUntilTimeout()
{
    ReliableSender sender;
    const TimePoint start = Clock::now();

    sender.enqueue(0x81, "a");
    sender.enqueue(0x82, "b");
    CHECK(sender.collect(start, 100ms, 0, 1200, 64).size() == 2);
    CHECK(sender.collect(start + 50ms, 100ms, 1, 1200, 64).empty());
    CHECK(sender.collect(start + 100ms, 100ms, 2, 1200, 64).size() == 2);
    CHECK(sender.outstanding() == 2);
}

void testRetransmissionKeepsTheRelId()
{
    ReliableSender sender;
    const TimePoint start = Clock::now();

    sender.enqueue(0x81, "a");
    const auto first = sender.collect(start, 100ms, 0, 1200, 64);
    const auto second = sender.collect(start + 200ms, 100ms, 1, 1200, 64);

    CHECK(first.size() == 1 && second.size() == 1);
    CHECK(first[0] == second[0]);
}

void testAckRemovesTheRecord()
{
    ReliableSender sender;
    const TimePoint start = Clock::now();

    sender.enqueue(0x81, "a");
    static_cast<void>(sender.collect(start, 100ms, 7, 1200, 64));
    sender.onAck(7, 0);
    CHECK(sender.pending() == 0);
    CHECK(sender.collect(start + 1s, 100ms, 8, 1200, 64).empty());
}

void testAckBitsAcknowledgeEarlierDatagrams()
{
    ReliableSender sender;
    const TimePoint start = Clock::now();

    sender.enqueue(0x81, "a");
    static_cast<void>(sender.collect(start, 100ms, 5, 1200, 64));
    sender.onAck(8, 0b100);
    CHECK(sender.pending() == 0);
}

void testThreeNewerAcksTriggerFastRetransmit()
{
    ReliableSender sender;
    const TimePoint start = Clock::now();

    sender.enqueue(0x81, "a");
    static_cast<void>(sender.collect(start, 10s, 0, 1200, 64));
    for (std::uint16_t seq = 1; seq <= 3; ++seq) {
        static_cast<void>(sender.collect(start, 10s, seq, 1200, 64));
        sender.onAck(seq, 0);
    }
    CHECK(sender.collect(start, 10s, 4, 1200, 64).size() == 1);
}

void testSendWindowIsLimited()
{
    ReliableSender sender;
    const TimePoint start = Clock::now();

    for (int i = 0; i < 300; ++i) {
        sender.enqueue(0x81, "x");
    }
    CHECK(sender.collect(start, 1s, 0, 100000, 1000).size() == 128);
    CHECK(sender.outstanding() == 128);
}

void testQueueOverflowIsReported()
{
    ReliableSender sender;

    for (std::size_t i = 0; i < MAX_RELIABLE_QUEUE; ++i) {
        CHECK(sender.enqueue(0x81, "x"));
    }
    CHECK(!sender.enqueue(0x81, "x"));
    CHECK(!sender.hasRoomToQueue());
}

void testDatagramSizeAndCountAreRespected()
{
    ReliableSender sender;
    const TimePoint start = Clock::now();

    for (int i = 0; i < 10; ++i) {
        sender.enqueue(0x81, std::string(10, 'x'));
    }
    CHECK(sender.collect(start, 1s, 0, 3 * 15, 64).size() == 3);
    CHECK(sender.collect(start, 1s, 1, 1200, 2).size() == 2);
}

struct LossyLink {
    std::mt19937 random{2024};
    int lossPercent = 30;
    int duplicatePercent = 10;

    int copies()
    {
        if (static_cast<int>(random() % 100) < lossPercent) {
            return 0;
        }
        return static_cast<int>(random() % 100) < duplicatePercent ? 2 : 1;
    }
};

void testReliableStreamSurvivesLossAndDuplication()
{
    ReliableSender sender;
    ReliableReceiver receiver;
    ReceiveWindow window;
    LossyLink link;
    std::vector<ReliableRecord> delivered;
    TimePoint now = Clock::now();
    std::uint16_t seq = 0;
    constexpr int TOTAL = 2000;

    for (int step = 0; step < 6000 && delivered.size() < TOTAL; ++step) {
        if (step < TOTAL) {
            sender.enqueue(0x81, std::to_string(step));
        }
        const std::uint16_t sent = seq++;
        const auto records = sender.collect(now, 100ms, sent, 1200, 64);

        for (int copy = link.copies(); copy > 0; --copy) {
            if (window.check(sent) != SeqVerdict::ACCEPT) {
                continue;
            }
            window.commit(sent);
            for (const ReliableRecord &r : records) {
                receiver.accept(r, delivered);
            }
        }
        if (window.hasReceived() && link.copies() > 0) {
            sender.onAck(window.latest(), window.seenMask());
        }
        now += 10ms;
    }
    CHECK(delivered.size() == TOTAL);
    for (std::size_t i = 0; i < delivered.size(); ++i) {
        CHECK(delivered[i].payload == std::to_string(i));
    }
}

int main()
{
    testInOrderDelivery();
    testOutOfOrderIsBufferedThenReleased();
    testDuplicatesAreIgnored();
    testWindowLimit();
    testRelIdWrapAround();
    testSenderSendsEachRecordOnceUntilTimeout();
    testRetransmissionKeepsTheRelId();
    testAckRemovesTheRecord();
    testAckBitsAcknowledgeEarlierDatagrams();
    testThreeNewerAcksTriggerFastRetransmit();
    testSendWindowIsLimited();
    testQueueOverflowIsReported();
    testDatagramSizeAndCountAreRespected();
    testReliableStreamSurvivesLossAndDuplication();
    return TEST_RESULT();
}
