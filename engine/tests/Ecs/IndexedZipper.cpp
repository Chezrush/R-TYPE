/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** IndexedZipper.cpp
*/

#include "Engine/Ecs/IndexedZipper.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <type_traits>
#include <vector>

#include "Engine/Ecs/EntityAllocator.hpp"
#include "Engine/Ecs/SparseArray.hpp"

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;
using engine::ecs::IndexedZipper;
using engine::ecs::makeIndexedZipper;
using engine::ecs::makeZipper;
using engine::ecs::SparseArray;

namespace {

struct Position {
    int x;
};

struct Velocity {
    int dx;
};

std::vector<Entity> makeEntities(std::size_t count)
{
    EntityAllocator allocator;
    std::vector<Entity> entities;

    for (std::size_t i = 0; i < count; ++i)
        entities.push_back(allocator.create());
    return entities;
}

} // namespace

TEST_CASE("an indexed zipper yields the index of each visited entity",
          "[ecs][zipper][indexed]")
{
    const auto entities = makeEntities(5);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;
    std::vector<std::size_t> indices;

    for (const Entity &entity : entities)
        positions.insert(entity, Position{0});
    velocities.insert(entities[1], Velocity{1});
    velocities.insert(entities[4], Velocity{1});

    for (auto [index, position, velocity] :
         IndexedZipper(positions, velocities)) {
        static_cast<void>(position);
        static_cast<void>(velocity);
        indices.push_back(index);
    }

    REQUIRE(indices == std::vector<std::size_t>{1, 4});
}

TEST_CASE("an indexed zipper skips entities like the plain zipper",
          "[ecs][zipper][indexed]")
{
    const auto entities = makeEntities(6);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;
    std::size_t plain = 0;
    std::size_t indexed = 0;

    for (std::size_t i = 0; i < 6; i += 2)
        positions.insert(entities[i], Position{0});
    for (std::size_t i = 0; i < 6; i += 4)
        velocities.insert(entities[i], Velocity{0});

    for (const auto &row : makeZipper(positions, velocities)) {
        static_cast<void>(row);
        ++plain;
    }
    for (const auto &row : makeIndexedZipper(positions, velocities)) {
        static_cast<void>(row);
        ++indexed;
    }

    REQUIRE(plain == 2);
    REQUIRE(indexed == plain);
}

TEST_CASE("an indexed zipper yields modifiable references",
          "[ecs][zipper][indexed]")
{
    const auto entities = makeEntities(3);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;

    positions.insert(entities[2], Position{1});
    velocities.insert(entities[2], Velocity{4});

    for (auto [index, position, velocity] :
         makeIndexedZipper(positions, velocities))
        position.x += velocity.dx + static_cast<int>(index);

    REQUIRE(positions.get(entities[2])->x == 7);
}

TEST_CASE("helpers deduce const-qualified arrays", "[ecs][zipper][indexed]")
{
    const auto entities = makeEntities(1);
    SparseArray<Position> positions;

    positions.insert(entities[0], Position{3});
    const auto &constPositions = positions;

    auto [index, position] = *makeIndexedZipper(constPositions).begin();

    STATIC_REQUIRE(
        std::is_const_v<std::remove_reference_t<decltype(position)>>);
    REQUIRE(index == 0);
    REQUIRE(position.x == 3);
}

TEST_CASE("an indexed zipper over empty arrays is empty",
          "[ecs][zipper][indexed]")
{
    SparseArray<Position> positions;
    const auto zipper = makeIndexedZipper(positions);

    REQUIRE(zipper.begin() == zipper.end());
}
