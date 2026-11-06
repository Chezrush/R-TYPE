/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetIdAllocator.cpp
*/

#include "Rtype/NetIdAllocator.hpp"

#include <catch2/catch_test_macros.hpp>
#include <set>

using rtype::game::NetIdAllocator;

namespace {

constexpr std::size_t CAPACITY = NetIdAllocator::CAPACITY;

void exhaust(NetIdAllocator &allocator)
{
    for (std::size_t i = 0; i < CAPACITY; ++i) {
        REQUIRE(allocator.allocate().has_value());
    }
}

} // namespace

TEST_CASE("a new allocator has no id in use", "[game][netid]")
{
    const NetIdAllocator allocator;

    REQUIRE(allocator.aliveCount() == 0);
    REQUIRE_FALSE(allocator.isAllocated(0));
    REQUIRE_FALSE(allocator.isAllocated(0xFFFF));
}

TEST_CASE("fresh ids are handed out in increasing order", "[game][netid]")
{
    NetIdAllocator allocator;

    REQUIRE(allocator.allocate() == 0);
    REQUIRE(allocator.allocate() == 1);
    REQUIRE(allocator.allocate() == 2);
    REQUIRE(allocator.aliveCount() == 3);
    REQUIRE(allocator.isAllocated(1));
}

TEST_CASE("a released id is no longer in use", "[game][netid]")
{
    NetIdAllocator allocator;
    const auto id = allocator.allocate();

    REQUIRE(allocator.release(*id));
    REQUIRE_FALSE(allocator.isAllocated(*id));
    REQUIRE(allocator.aliveCount() == 0);
}

TEST_CASE("a released id is not reused while fresh ids remain", "[game][netid]")
{
    NetIdAllocator allocator;
    const auto first = allocator.allocate();

    REQUIRE(allocator.release(*first));
    REQUIRE(allocator.allocate() != first);
}

TEST_CASE("all the ids are different and 0xFFFF is usable", "[game][netid]")
{
    NetIdAllocator allocator;
    std::set<std::uint16_t> seen;

    for (std::size_t i = 0; i < CAPACITY; ++i) {
        seen.insert(*allocator.allocate());
    }
    REQUIRE(seen.size() == CAPACITY);
    REQUIRE(allocator.isAllocated(0xFFFF));
}

TEST_CASE("running out of ids returns nothing instead of failing",
          "[game][netid]")
{
    NetIdAllocator allocator;

    exhaust(allocator);
    REQUIRE(allocator.aliveCount() == CAPACITY);
    REQUIRE_FALSE(allocator.allocate().has_value());
    REQUIRE_FALSE(allocator.allocate().has_value());
    REQUIRE(allocator.aliveCount() == CAPACITY);
}

TEST_CASE("a full allocator gives an id back once one is released",
          "[game][netid]")
{
    NetIdAllocator allocator;

    exhaust(allocator);
    REQUIRE(allocator.release(1234));
    REQUIRE(allocator.allocate() == 1234);
    REQUIRE_FALSE(allocator.allocate().has_value());
}

TEST_CASE("released ids come back oldest first", "[game][netid]")
{
    NetIdAllocator allocator;

    exhaust(allocator);
    REQUIRE(allocator.release(50));
    REQUIRE(allocator.release(7));
    REQUIRE(allocator.release(900));

    REQUIRE(allocator.allocate() == 50);
    REQUIRE(allocator.allocate() == 7);
    REQUIRE(allocator.allocate() == 900);
}

TEST_CASE("releasing an id twice or an unknown id changes nothing",
          "[game][netid]")
{
    NetIdAllocator allocator;
    const auto id = allocator.allocate();

    REQUIRE_FALSE(allocator.release(500));
    REQUIRE(allocator.release(*id));
    REQUIRE_FALSE(allocator.release(*id));
    REQUIRE(allocator.aliveCount() == 0);
}

TEST_CASE("a double release never hands the same id to two entities",
          "[game][netid]")
{
    NetIdAllocator allocator;

    exhaust(allocator);
    REQUIRE(allocator.release(42));
    REQUIRE_FALSE(allocator.release(42));

    REQUIRE(allocator.allocate() == 42);
    REQUIRE_FALSE(allocator.allocate().has_value());
}
