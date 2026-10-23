/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Access.cpp
*/

#include "Engine/Ecs/Access.hpp"

#include <catch2/catch_test_macros.hpp>
#include <typeindex>

using engine::ecs::Access;

namespace {

struct Position {
    int x;
};

struct Velocity {
    int dx;
};

struct Health {
    int points;
};

std::type_index typeOf(const std::type_info &info)
{
    return std::type_index(info);
}

} // namespace

TEST_CASE("a new access declares nothing", "[ecs][access]")
{
    const Access access;

    REQUIRE(access.reads().empty());
    REQUIRE(access.writes().empty());
}

TEST_CASE("reads and writes are recorded by component type", "[ecs][access]")
{
    Access access;

    access.read<Position>().write<Velocity>();

    REQUIRE(access.reads().size() == 1);
    REQUIRE(access.reads().contains(typeOf(typeid(Position))));
    REQUIRE(access.writes().size() == 1);
    REQUIRE(access.writes().contains(typeOf(typeid(Velocity))));
}

TEST_CASE("declaring a type twice keeps a single entry", "[ecs][access]")
{
    Access access;

    access.read<Position>()
        .read<Position>()
        .write<Velocity>()
        .write<Velocity>();

    REQUIRE(access.reads().size() == 1);
    REQUIRE(access.writes().size() == 1);
}

TEST_CASE("two writes of one type conflict", "[ecs][access]")
{
    Access first;
    Access second;

    first.write<Position>();
    second.write<Position>();

    REQUIRE(first.conflictsWith(second));
    REQUIRE(second.conflictsWith(first));
}

TEST_CASE("a read and a write of one type conflict in both directions",
          "[ecs][access]")
{
    Access reader;
    Access writer;

    reader.read<Position>();
    writer.write<Position>();

    REQUIRE(reader.conflictsWith(writer));
    REQUIRE(writer.conflictsWith(reader));
}

TEST_CASE("two reads of one type do not conflict", "[ecs][access]")
{
    Access first;
    Access second;

    first.read<Position>();
    second.read<Position>();

    REQUIRE_FALSE(first.conflictsWith(second));
}

TEST_CASE("accesses on different types do not conflict", "[ecs][access]")
{
    Access first;
    Access second;

    first.read<Position>().write<Velocity>();
    second.read<Health>().write<Health>();

    REQUIRE_FALSE(first.conflictsWith(second));
    REQUIRE_FALSE(second.conflictsWith(first));
}

TEST_CASE("an empty access never conflicts", "[ecs][access]")
{
    Access busy;

    busy.write<Position>().read<Velocity>();

    REQUIRE_FALSE(busy.conflictsWith(Access()));
    REQUIRE_FALSE(Access().conflictsWith(busy));
}

TEST_CASE("a system does not conflict with itself on its own reads",
          "[ecs][access]")
{
    Access access;

    access.read<Position>().read<Velocity>();

    REQUIRE_FALSE(access.conflictsWith(access));
}
