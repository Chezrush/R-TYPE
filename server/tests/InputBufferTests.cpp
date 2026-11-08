/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** InputBufferTests
*/

#include <cstdint>
#include <optional>
#include <vector>

#include "InputBuffer.hpp"
#include "TestSupport.hpp"

using rtype::protocol::Input;
using rtype::server::InputBuffer;

namespace {

void testSamplesArePoppedInOrder()
{
    InputBuffer buffer;

    buffer.push(Input{10, {1, 2, 3}});
    CHECK(buffer.pop() == 1);
    CHECK(buffer.pop() == 2);
    CHECK(buffer.lastApplied() == 11);
    CHECK(buffer.pop() == 3);
    CHECK(buffer.pop() == std::nullopt);
    CHECK(buffer.lastApplied() == 12);
}

void testAppliedSamplesAreIgnored()
{
    InputBuffer buffer;

    buffer.push(Input{0, {1, 2}});
    CHECK(buffer.pop() == 1);
    buffer.push(Input{0, {1, 2, 3}});
    CHECK(buffer.pop() == 2);
    CHECK(buffer.pop() == 3);
    CHECK(buffer.pop() == std::nullopt);
}

void testMissingIdsAreSkipped()
{
    InputBuffer buffer;

    buffer.push(Input{0, {1}});
    buffer.push(Input{3, {4}});
    CHECK(buffer.pop() == 1);
    CHECK(buffer.pop() == 4);
    CHECK(buffer.lastApplied() == 3);
}

void testAtMostEightSamplesArePending()
{
    const std::vector<std::uint8_t> samples = {0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<std::uint8_t> popped;
    InputBuffer buffer;

    buffer.push(Input{0, samples});
    buffer.push(Input{8, {8}});
    while (const auto keys = buffer.pop()) {
        popped.push_back(*keys);
    }
    CHECK(popped == samples);
    buffer.push(Input{8, {8}});
    CHECK(buffer.pop() == 8);
}

void testIdsWrapAround()
{
    InputBuffer buffer;

    buffer.push(Input{65535, {1, 2}});
    CHECK(buffer.pop() == 1);
    CHECK(buffer.pop() == 2);
    CHECK(buffer.lastApplied() == 0);
}

} // namespace

int main()
{
    testSamplesArePoppedInOrder();
    testAppliedSamplesAreIgnored();
    testMissingIdsAreSkipped();
    testAtMostEightSamplesArePending();
    testIdsWrapAround();

    return TEST_RESULT();
}
