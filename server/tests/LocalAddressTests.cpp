/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LocalAddressTests.cpp
*/

#include <string>

#include "LocalAddress.hpp"
#include "TestSupport.hpp"

using namespace rtype::server;

namespace {

void testReachableAddresses()
{
    CHECK(isReachableAddress("192.168.1.20"));
    CHECK(isReachableAddress("10.0.0.5"));
    CHECK(!isReachableAddress("127.0.0.1"));
    CHECK(!isReachableAddress("0.0.0.0"));
    CHECK(!isReachableAddress("::1"));
    CHECK(!isReachableAddress("not an address"));
    CHECK(!isReachableAddress(""));
}

void testListedAddressesAreUsable()
{
    const auto addresses = localAddresses();

    for (const std::string &address : addresses) {
        CHECK(isReachableAddress(address));
    }
}

void testNoDuplicate()
{
    const auto addresses = localAddresses();

    for (std::size_t i = 0; i < addresses.size(); ++i) {
        for (std::size_t j = i + 1; j < addresses.size(); ++j) {
            CHECK(addresses[i] != addresses[j]);
        }
    }
}

} // namespace

int main()
{
    testReachableAddresses();
    testListedAddressesAreUsable();
    testNoDuplicate();
    return TEST_RESULT();
}
