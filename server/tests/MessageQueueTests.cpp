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

constexpr unsigned int PRODUCER_COUNT = 4;
constexpr unsigned int MESSAGES_PER_PRODUCER = 1000;
constexpr unsigned int EXPECTED_TOTAL = PRODUCER_COUNT * MESSAGES_PER_PRODUCER;

void testEnqueueDequeue()
{
    rtype::server::MessageQueue<int> queue(4);

    CHECK(queue.handleEnqueue(1));
    CHECK(queue.handleEnqueue(2));
    CHECK(queue.handleEnqueue(3));
    CHECK(queue.size() == 3);

    const auto first = queue.handleDequeue();
    const auto second = queue.handleDequeue();
    const auto third = queue.handleDequeue();

    CHECK(first.has_value());
    CHECK(second.has_value());
    CHECK(third.has_value());
    CHECK(first.value() == 1);
    CHECK(second.value() == 2);
    CHECK(third.value() == 3);
    CHECK(queue.empty());
}

void testCapacityLimit()
{
    rtype::server::MessageQueue<int> queue(2);

    CHECK(queue.handleEnqueue(1));
    CHECK(queue.handleEnqueue(2));
    CHECK(queue.full());
    CHECK(!queue.handleEnqueue(3));
    CHECK(queue.size() == 2);

    CHECK(queue.handleDequeue().has_value());
    CHECK(!queue.full());
    CHECK(queue.handleEnqueue(3));
}

void testEmptyQueueReports()
{
    rtype::server::MessageQueue<int> queue(2);

    CHECK(queue.empty());
    CHECK(!queue.handleDequeue().has_value());
}

void produceMessages(rtype::server::MessageQueue<unsigned int> &queue,
                     unsigned int producerId)
{
    for (unsigned int index = 0; index < MESSAGES_PER_PRODUCER; ++index) {
        while (!queue.handleEnqueue(producerId)) {
            std::this_thread::yield();
        }
    }
}

unsigned int consumeMessages(rtype::server::MessageQueue<unsigned int> &queue)
{
    unsigned int received = 0;

    while (received < EXPECTED_TOTAL) {
        if (queue.handleDequeue().has_value()) {
            ++received;
        } else {
            std::this_thread::yield();
        }
    }
    return received;
}

void testConcurrentAccess()
{
    rtype::server::MessageQueue<unsigned int> queue(EXPECTED_TOTAL);
    std::vector<std::jthread> producers;

    for (unsigned int producer = 0; producer < PRODUCER_COUNT; ++producer) {
        producers.emplace_back(
            [&queue, producer]() { produceMessages(queue, producer); });
    }

    const auto received = consumeMessages(queue);

    for (auto &producer : producers) {
        producer.join();
    }

    CHECK(received == EXPECTED_TOTAL);
    CHECK(queue.empty());
}

int main()
{
    testEnqueueDequeue();
    testCapacityLimit();
    testEmptyQueueReports();
    testConcurrentAccess();

    return TEST_RESULT();
}
