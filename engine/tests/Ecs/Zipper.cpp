/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Zipper.cpp
*/

#include "Engine/Ecs/Zipper.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>

#include "Engine/Ecs/EntityAllocator.hpp"
#include "Engine/Ecs/SparseArray.hpp"

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;
using engine::ecs::SparseArray;
using engine::ecs::Zipper;

namespace {

struct Position {
    int x;
};

struct Velocity {
    int dx;
};

struct Tag {
    int id;
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

TEST_CASE("a zipper visits only the entities owning every component",
          "[ecs][zipper]")
{
    const auto entities = makeEntities(5);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;
    std::vector<int> visited;

    for (std::size_t i = 0; i < 5; ++i)
        positions.insert(entities[i], Position{static_cast<int>(i)});
    velocities.insert(entities[1], Velocity{10});
    velocities.insert(entities[3], Velocity{30});
    velocities.insert(entities[4], Velocity{40});
    positions.erase(entities[4]);

    for (auto [position, velocity] : Zipper(positions, velocities))
        visited.push_back(position.x + velocity.dx);

    REQUIRE(visited == std::vector<int>{11, 33});
}

TEST_CASE("a zipper yields references to the stored components",
          "[ecs][zipper]")
{
    const auto entities = makeEntities(2);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;

    positions.insert(entities[1], Position{1});
    velocities.insert(entities[1], Velocity{5});

    for (auto [position, velocity] : Zipper(positions, velocities))
        position.x += velocity.dx;

    REQUIRE(positions.get(entities[1])->x == 6);
}

TEST_CASE("a zipper over const arrays yields const references", "[ecs][zipper]")
{
    const auto entities = makeEntities(1);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;

    positions.insert(entities[0], Position{2});
    velocities.insert(entities[0], Velocity{3});
    const auto &constPositions = positions;
    const auto &constVelocities = velocities;
    const Zipper zipper(constPositions, constVelocities);

    auto [position, velocity] = *zipper.begin();

    STATIC_REQUIRE(
        std::is_const_v<std::remove_reference_t<decltype(position)>>);
    REQUIRE(position.x + velocity.dx == 5);
}

TEST_CASE("a zipper stops at the shortest array", "[ecs][zipper]")
{
    const auto entities = makeEntities(4);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;
    std::size_t count = 0;

    for (const Entity &entity : entities)
        positions.insert(entity, Position{0});
    velocities.insert(entities[1], Velocity{1});

    for (const auto &row : Zipper(positions, velocities)) {
        static_cast<void>(row);
        ++count;
    }

    REQUIRE(count == 1);
}

TEST_CASE("a zipper over empty arrays is empty", "[ecs][zipper]")
{
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;
    const Zipper zipper(positions, velocities);

    REQUIRE(zipper.begin() == zipper.end());
}

TEST_CASE("a zipper over three arrays needs all three components",
          "[ecs][zipper]")
{
    const auto entities = makeEntities(3);
    SparseArray<Position> positions;
    SparseArray<Velocity> velocities;
    SparseArray<Tag> tags;
    std::size_t count = 0;

    for (const Entity &entity : entities) {
        positions.insert(entity, Position{0});
        velocities.insert(entity, Velocity{0});
    }
    tags.insert(entities[2], Tag{7});

    for (auto [position, velocity, tag] : Zipper(positions, velocities, tags))
        count += static_cast<std::size_t>(tag.id);

    REQUIRE(count == 7);
}

TEST_CASE("a zipper over one array skips the empty slots", "[ecs][zipper]")
{
    const auto entities = makeEntities(3);
    SparseArray<Position> positions;
    std::size_t count = 0;

    positions.insert(entities[2], Position{1});

    for (auto [position] : Zipper(positions))
        count += static_cast<std::size_t>(position.x);

    REQUIRE(count == 1);
}

TEST_CASE("post-increment returns the previous position", "[ecs][zipper]")
{
    const auto entities = makeEntities(2);
    SparseArray<Position> positions;

    positions.insert(entities[0], Position{1});
    positions.insert(entities[1], Position{2});
    const Zipper zipper(positions);

    auto it = zipper.begin();
    auto [first] = *it++;

    REQUIRE(first.x == 1);
    REQUIRE(std::get<0>(*it).x == 2);
}
