/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MessageQueueTests.cpp
*/

#include <thread>
#include <vector>

#include "MessageQueue.hpp"
#include "ServerMessage.hpp"
#include "TestSupport.hpp"

namespace {

using rtype::server::IncomingMessage;
using rtype::server::MessageQueue;

void testEnqueueDequeueOrder()
{
    MessageQueue<int> queue(4);

    CHECK(queue.handleEnqueue(1) == true);
    CHECK(queue.handleEnqueue(2) == true);
    CHECK(queue.handleEnqueue(3) == true);

    CHECK(queue.size() == 3);

    const auto first = queue.handleDequeue();
    const auto second = queue.handleDequeue();
    const auto third = queue.handleDequeue();

    CHECK(first.has_value() == true);
    CHECK(second.has_value() == true);
    CHECK(third.has_value() == true);
    CHECK(*first == 1);
    CHECK(*second == 2);
    CHECK(*third == 3);
    CHECK(queue.empty() == true);
}

void testCapacityLimit()
{
    MessageQueue<int> queue(2);

    CHECK(queue.handleEnqueue(1) == true);
    CHECK(queue.handleEnqueue(2) == true);
    CHECK(queue.full() == true);
    CHECK(queue.handleEnqueue(3) == false);
    CHECK(queue.size() == 2);

    CHECK(queue.handleDequeue().has_value() == true);
    CHECK(queue.full() == false);
    CHECK(queue.handleEnqueue(3) == true);
}

void testEmptyQueueReportsWithoutBlocking()
{
    MessageQueue<int> queue(2);

    CHECK(queue.empty() == true);
    CHECK(queue.handleDequeue().has_value() == false);
}

void testConcurrentAccess()
{
    constexpr unsigned int producerCount = 4;
    constexpr unsigned int messagesPerProducer = 1000;
    constexpr unsigned int expectedTotal = producerCount * messagesPerProducer;

    MessageQueue<unsigned int> queue(expectedTotal);
    std::vector<std::jthread> producers;

    for (unsigned int producer = 0; producer < producerCount; ++producer) {
        producers.emplace_back([&queue, producer]() {
            for (unsigned int index = 0; index < messagesPerProducer; ++index) {
                while (!queue.handleEnqueue(producer)) {
                    std::this_thread::yield();
                }
            }
        });
    }

    unsigned int received = 0;

    while (received < expectedTotal) {
        if (queue.handleDequeue().has_value()) {
            ++received;
        } else {
            std::this_thread::yield();
        }
    }

    for (auto &producer : producers) {
        producer.join();
    }

    CHECK(received == expectedTotal);
    CHECK(queue.empty() == true);
}

} /* namespace */

int main()
{
    testEnqueueDequeueOrder();
    testCapacityLimit();
    testEmptyQueueReportsWithoutBlocking();
    testConcurrentAccess();

    return TEST_RESULT();
}
