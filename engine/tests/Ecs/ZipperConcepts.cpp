/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ZipperConcepts.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <vector>

#include "Engine/Ecs/EntityAllocator.hpp"
#include "Engine/Ecs/IndexedZipper.hpp"
#include "Engine/Ecs/SparseArray.hpp"
#include "Engine/Ecs/Zipper.hpp"

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;
using engine::ecs::IndexedZipper;
using engine::ecs::IndexedZipperIterator;
using engine::ecs::SparseArray;
using engine::ecs::Zipper;
using engine::ecs::ZipperIterator;

namespace {

struct Position {
    int x;
};

struct Velocity {
    int dx;
};

using Positions = SparseArray<Position>;
using Velocities = SparseArray<Velocity>;

static_assert(std::input_iterator<ZipperIterator<Positions>>);
static_assert(std::input_iterator<ZipperIterator<Positions, Velocities>>);
static_assert(std::input_iterator<ZipperIterator<const Positions>>);
static_assert(std::input_iterator<ZipperIterator<const Positions, Velocities>>);
static_assert(std::input_iterator<IndexedZipperIterator<Positions>>);
static_assert(
    std::input_iterator<IndexedZipperIterator<Positions, Velocities>>);
static_assert(std::input_iterator<IndexedZipperIterator<const Positions>>);
static_assert(std::ranges::input_range<Zipper<Positions, Velocities>>);
static_assert(std::ranges::input_range<Zipper<const Positions>>);
static_assert(std::ranges::input_range<IndexedZipper<Positions, Velocities>>);
static_assert(std::ranges::input_range<IndexedZipper<const Positions>>);

std::vector<Entity> makeEntities(std::size_t count)
{
    EntityAllocator allocator;
    std::vector<Entity> entities;

    for (std::size_t i = 0; i < count; ++i)
        entities.push_back(allocator.create());
    return entities;
}

template <typename Range>
std::size_t countElements(const Range &range)
{
    std::size_t count = 0;

    for (const auto &element : range) {
        static_cast<void>(element);
        ++count;
    }
    return count;
}

} // namespace

TEST_CASE("arrays with no common entity yield an empty range",
          "[ecs][zipper][edge]")
{
    const auto entities = makeEntities(4);
    Positions positions;
    Velocities velocities;

    positions.insert(entities[0], Position{0});
    positions.insert(entities[2], Position{0});
    velocities.insert(entities[1], Velocity{0});
    velocities.insert(entities[3], Velocity{0});

    REQUIRE(countElements(Zipper(positions, velocities)) == 0);
    REQUIRE(countElements(IndexedZipper(positions, velocities)) == 0);
}

TEST_CASE("one empty array makes the whole range empty", "[ecs][zipper][edge]")
{
    const auto entities = makeEntities(3);
    Positions positions;
    Velocities velocities;

    for (const Entity &entity : entities)
        positions.insert(entity, Position{0});

    REQUIRE(countElements(Zipper(positions, velocities)) == 0);
    REQUIRE(countElements(IndexedZipper(velocities, positions)) == 0);
}

TEST_CASE("holes in the arrays are skipped on both variants",
          "[ecs][zipper][edge]")
{
    const auto entities = makeEntities(7);
    Positions positions;
    Velocities velocities;
    std::vector<std::size_t> plain;
    std::vector<std::size_t> indexed;

    positions.insert(entities[0], Position{0});
    positions.insert(entities[3], Position{3});
    positions.insert(entities[6], Position{6});
    velocities.insert(entities[3], Velocity{0});
    velocities.insert(entities[6], Velocity{0});
    for (auto [position, velocity] : Zipper(positions, velocities)) {
        static_cast<void>(velocity);
        plain.push_back(static_cast<std::size_t>(position.x));
    }
    for (auto [index, position, velocity] :
         IndexedZipper(positions, velocities)) {
        static_cast<void>(position);
        static_cast<void>(velocity);
        indexed.push_back(index);
    }

    REQUIRE(plain == std::vector<std::size_t>{3, 6});
    REQUIRE(indexed == plain);
}

TEST_CASE("range-for with structured bindings works on const arrays",
          "[ecs][zipper][edge]")
{
    const auto entities = makeEntities(3);
    Positions positions;
    Velocities velocities;
    int sum = 0;

    for (const Entity &entity : entities) {
        positions.insert(entity, Position{1});
        velocities.insert(entity, Velocity{2});
    }
    const Positions &constPositions = positions;
    const Velocities &constVelocities = velocities;

    for (const auto &[position, velocity] :
         Zipper(constPositions, constVelocities))
        sum += position.x + velocity.dx;

    REQUIRE(sum == 9);
}

TEST_CASE("range-for with structured bindings works on mixed constness",
          "[ecs][zipper][edge]")
{
    const auto entities = makeEntities(2);
    Positions positions;
    Velocities velocities;

    for (const Entity &entity : entities) {
        positions.insert(entity, Position{0});
        velocities.insert(entity, Velocity{3});
    }
    const Velocities &constVelocities = velocities;

    for (auto [index, position, velocity] :
         IndexedZipper(positions, constVelocities))
        position.x = velocity.dx + static_cast<int>(index);

    REQUIRE(positions.get(entities[0])->x == 3);
    REQUIRE(positions.get(entities[1])->x == 4);
}

TEST_CASE("std algorithms accept the zipper range", "[ecs][zipper][edge]")
{
    const auto entities = makeEntities(4);
    Positions positions;

    for (const Entity &entity : entities)
        positions.insert(entity, Position{1});
    positions.erase(entities[1]);
    const Zipper zipper(positions);

    REQUIRE(std::distance(zipper.begin(), zipper.end()) == 3);
}
