/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SecurityTests.cpp
*/

#include <string>

#include "CookieFactory.hpp"
#include "RateLimiter.hpp"
#include "SipHash.hpp"
#include "TestSupport.hpp"

using namespace rtype::protocol;
using namespace std::chrono_literals;

SipKey referenceKey()
{
    SipKey key{};

    for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<std::uint8_t>(i);
    }
    return key;
}

void testSipHashReferenceVectors()
{
    const std::string one("\x00", 1);
    std::string fifteen;

    for (int i = 0; i < 15; ++i) {
        fifteen += static_cast<char>(i);
    }
    CHECK(sipHash24(referenceKey(), "") == 0x726fdb47dd0e0e31ULL);
    CHECK(sipHash24(referenceKey(), one) == 0x74f839c593dc67fdULL);
    CHECK(sipHash24(referenceKey(), fifteen) == 0xa129ca6149be45e5ULL);
}

void testCookieIsBoundToTheAddress()
{
    const CookieFactory factory(referenceKey());
    const auto cookie = factory.make("10.0.0.1:4000", 100);

    CHECK(cookie != 0);
    CHECK(factory.verify("10.0.0.1:4000", cookie, 100));
    CHECK(!factory.verify("10.0.0.2:4000", cookie, 100));
    CHECK(!factory.verify("10.0.0.1:4001", cookie, 100));
}

void testCookieLifetime()
{
    const CookieFactory factory(referenceKey());
    const auto cookie = factory.make("a", 100);

    CHECK(factory.verify("a", cookie, 104));
    CHECK(factory.verify("a", cookie, 109));
    CHECK(!factory.verify("a", cookie, 110));
}

void testCookieNeedsTheSecret()
{
    SipKey other = referenceKey();
    other[0] ^= 1;
    const CookieFactory first(referenceKey());
    const CookieFactory second(other);

    CHECK(!second.verify("a", first.make("a", 7), 7));
    CHECK(!first.verify("a", 0, 7));
}

void testRandomSecretsDiffer()
{
    CHECK(CookieFactory::randomSecret() != CookieFactory::randomSecret());
}

void testRateLimiterBurstThenRefill()
{
    RateLimiter limiter(5.0, 3.0, 16);
    const TimePoint start = Clock::now();

    CHECK(limiter.allow("a", start));
    CHECK(limiter.allow("a", start));
    CHECK(limiter.allow("a", start));
    CHECK(!limiter.allow("a", start));
    CHECK(limiter.allow("b", start));
    CHECK(!limiter.allow("a", start + 100ms));
    CHECK(limiter.allow("a", start + 250ms));
}

void testRateLimiterTableIsBounded()
{
    RateLimiter limiter(1.0, 1.0, 4);
    const TimePoint start = Clock::now();

    for (int i = 0; i < 4; ++i) {
        CHECK(limiter.allow("key" + std::to_string(i), start));
    }
    CHECK(!limiter.allow("another", start));
    CHECK(limiter.size() == 4);
    CHECK(limiter.allow("another", start + 2s));
    CHECK(limiter.size() == 1);
}

int main()
{
    testSipHashReferenceVectors();
    testCookieIsBoundToTheAddress();
    testCookieLifetime();
    testCookieNeedsTheSecret();
    testRandomSecretsDiffer();
    testRateLimiterBurstThenRefill();
    testRateLimiterTableIsBounded();
    return TEST_RESULT();
}
