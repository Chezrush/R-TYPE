/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NumericTests.cpp
*/

#include <cmath>
#include <limits>

#include "FixedPoint.hpp"
#include "SerialNumber.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;

void testFixedPointRoundTrip()
{
    CHECK(toFixed(80.0F) == 1280);
    CHECK(toFixed(-16.0F) == -256);
    CHECK(fromFixed(1280) == 80.0F);
    CHECK(fromFixed(toFixed(12.0625F)) == 12.0625F);
}

void testFixedPointRounding()
{
    CHECK(toFixed(0.03F) == 0);
    CHECK(toFixed(0.04F) == 1);
    CHECK(toFixed(-0.04F) == -1);
}

void testFixedPointSaturates()
{
    CHECK(toFixed(1e9F) == std::numeric_limits<std::int16_t>::max());
    CHECK(toFixed(-1e9F) == std::numeric_limits<std::int16_t>::min());
    CHECK(toFixed(std::numeric_limits<float>::infinity()) == 32767);
    CHECK(toFixed(std::nanf("")) == 0);
}

void testSerialNewer()
{
    CHECK(isNewer(std::uint16_t{5}, std::uint16_t{4}));
    CHECK(!isNewer(std::uint16_t{4}, std::uint16_t{5}));
    CHECK(!isNewer(std::uint16_t{4}, std::uint16_t{4}));
}

void testSerialWrapAround()
{
    CHECK(isNewer(std::uint16_t{0}, std::uint16_t{65535}));
    CHECK(isNewer(std::uint16_t{10}, std::uint16_t{65530}));
    CHECK(!isNewer(std::uint16_t{65530}, std::uint16_t{10}));
    CHECK(isNewer(std::uint32_t{1}, std::uint32_t{0xFFFFFFFFU}));
}

void testSerialHalfRange()
{
    CHECK(isNewer(std::uint16_t{0x7FFF}, std::uint16_t{0}));
    CHECK(!isNewer(std::uint16_t{0x8000}, std::uint16_t{0}));
    CHECK(distance(std::uint16_t{2}, std::uint16_t{65534}) == 4);
    CHECK(distance(std::uint32_t{1}, std::uint32_t{0xFFFFFFFFU}) == 2);
}

int main()
{
    testFixedPointRoundTrip();
    testFixedPointRounding();
    testFixedPointSaturates();
    testSerialNewer();
    testSerialWrapAround();
    testSerialHalfRange();
    return TEST_RESULT();
}
