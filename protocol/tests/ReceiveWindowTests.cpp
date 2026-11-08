/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReceiveWindowTests.cpp
*/

#include <set>

#include "ReceiveWindow.hpp"
#include "RttEstimator.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;
using namespace std::chrono_literals;

bool receive(ReceiveWindow &window, std::uint16_t seq)
{
    if (window.check(seq) != SeqVerdict::ACCEPT) {
        return false;
    }
    window.commit(seq);
    return true;
}

void testFirstDatagramFixesTheWindow()
{
    ReceiveWindow window;

    CHECK(!window.hasReceived());
    CHECK(receive(window, 40000));
    CHECK(window.hasReceived());
    CHECK(window.latest() == 40000);
    CHECK(window.seenMask() == 0);
}

void testDuplicateIsDropped()
{
    ReceiveWindow window;

    CHECK(receive(window, 5));
    CHECK(window.check(5) == SeqVerdict::DUPLICATE);
    CHECK(receive(window, 6));
    CHECK(window.check(5) == SeqVerdict::DUPLICATE);
    CHECK(window.check(6) == SeqVerdict::DUPLICATE);
}

void testReorderedDatagramIsAcceptedOnce()
{
    ReceiveWindow window;

    CHECK(receive(window, 10));
    CHECK(receive(window, 14));
    CHECK(receive(window, 12));
    CHECK(!receive(window, 12));
    CHECK(receive(window, 11));
    CHECK(window.latest() == 14);
}

void testAckBitsDescribeThePreviousDatagrams()
{
    ReceiveWindow window;

    for (std::uint16_t seq : {10, 11, 13, 14}) {
        CHECK(receive(window, seq));
    }
    CHECK(window.latest() == 14);
    CHECK(window.seenMask() == 0b1101);
}

void testTooOldDatagramIsDropped()
{
    ReceiveWindow window;

    CHECK(receive(window, 100));
    CHECK(receive(window, 200));
    CHECK(window.check(100) == SeqVerdict::TOO_OLD);
    CHECK(window.check(168) == SeqVerdict::ACCEPT);
    CHECK(window.check(167) == SeqVerdict::TOO_OLD);
}

void testSequenceWrapAround()
{
    ReceiveWindow window;

    CHECK(receive(window, 65534));
    CHECK(receive(window, 65535));
    CHECK(receive(window, 1));
    CHECK(receive(window, 0));
    CHECK(!receive(window, 65535));
    CHECK(window.latest() == 1);
}

void testHugeJumpClearsTheMask()
{
    ReceiveWindow window;

    CHECK(receive(window, 1));
    CHECK(receive(window, 30000));
    CHECK(window.seenMask() == 0);
    CHECK(window.check(1) == SeqVerdict::TOO_OLD);
}

void testEveryDatagramIsDeliveredOnceUnderDuplication()
{
    ReceiveWindow window;
    std::multiset<std::uint16_t> delivered;

    for (std::uint16_t seq = 65500; seq != 200; ++seq) {
        for (int copy = 0; copy < 3; ++copy) {
            if (receive(window, seq)) {
                delivered.insert(seq);
            }
        }
    }
    CHECK(delivered.size() == 236);
    CHECK(delivered.count(65535) == 1);
}

void testRttFirstSampleThenSmoothing()
{
    RttEstimator rtt;

    CHECK(!rtt.hasSample());
    CHECK(rtt.retransmissionTimeout() == 1s);
    rtt.addSample(80ms);
    CHECK(rtt.smoothed() == 80ms);
    rtt.addSample(160ms);
    CHECK(rtt.smoothed() == 90ms);
    CHECK(rtt.retransmissionTimeout() == 180ms);
}

void testRetransmissionTimeoutHasAFloor()
{
    RttEstimator rtt;

    rtt.addSample(5ms);
    CHECK(rtt.retransmissionTimeout() == 100ms);
    rtt.addSample(-5ms);
    CHECK(rtt.smoothed() == 5ms);
}

void testRetransmissionTimeoutHasACeiling()
{
    RttEstimator rtt;

    rtt.addSample(5s);
    CHECK(rtt.retransmissionTimeout() == 3s);
}

int main()
{
    testFirstDatagramFixesTheWindow();
    testDuplicateIsDropped();
    testReorderedDatagramIsAcceptedOnce();
    testAckBitsDescribeThePreviousDatagrams();
    testTooOldDatagramIsDropped();
    testSequenceWrapAround();
    testHugeJumpClearsTheMask();
    testEveryDatagramIsDeliveredOnceUnderDuplication();
    testRttFirstSampleThenSmoothing();
    testRetransmissionTimeoutHasAFloor();
    testRetransmissionTimeoutHasACeiling();
    return TEST_RESULT();
}
