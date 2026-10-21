/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SparseArray.cpp
*/

#include "Engine/Ecs/SparseArray.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>

#include "Engine/Ecs/EntityAllocator.hpp"

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;
using engine::ecs::IStorage;
using engine::ecs::SparseArray;

namespace {

struct Position {
    float x;
    float y;
};

template <typename T>
T makeValue(int seed);

template <>
Position makeValue<Position>(int seed)
{
    return Position{static_cast<float>(seed), 0.F};
}

template <>
std::string makeValue<std::string>(int seed)
{
    return std::string(40, 'a') + std::to_string(seed);
}

template <>
std::unique_ptr<int> makeValue<std::unique_ptr<int>>(int seed)
{
    return std::make_unique<int>(seed);
}

} // namespace

TEMPLATE_TEST_CASE("a new array holds nothing", "[ecs][sparse]", Position,
                   std::string, std::unique_ptr<int>)
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();
    const SparseArray<TestType> array;

    REQUIRE(array.size() == 0);
    REQUIRE_FALSE(array.has(entity));
    REQUIRE(array.get(entity) == nullptr);
}

TEMPLATE_TEST_CASE("a component is stored at the entity index", "[ecs][sparse]",
                   Position, std::string, std::unique_ptr<int>)
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    const Entity second = allocator.create();
    SparseArray<TestType> array;

    array.insert(second, makeValue<TestType>(2));

    REQUIRE(array.size() == 2);
    REQUIRE(array.has(second));
    REQUIRE_FALSE(array.has(first));
    REQUIRE(array.get(first) == nullptr);
    REQUIRE(array.get(second) != nullptr);
}

TEMPLATE_TEST_CASE("inserting again replaces the component", "[ecs][sparse]",
                   Position, std::string, std::unique_ptr<int>)
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();
    SparseArray<TestType> array;

    array.insert(entity, makeValue<TestType>(1));
    TestType &replaced = array.insert(entity, makeValue<TestType>(2));

    REQUIRE(array.size() == 1);
    REQUIRE(&replaced == array.get(entity));
}

TEMPLATE_TEST_CASE("erasing removes only the component of the entity",
                   "[ecs][sparse]", Position, std::string, std::unique_ptr<int>)
{
    EntityAllocator allocator;
    const Entity first = allocator.create();
    const Entity second = allocator.create();
    SparseArray<TestType> array;
    array.insert(first, makeValue<TestType>(1));
    array.insert(second, makeValue<TestType>(2));

    array.erase(first);

    REQUIRE_FALSE(array.has(first));
    REQUIRE(array.has(second));
}

TEMPLATE_TEST_CASE("erasing without a component does nothing", "[ecs][sparse]",
                   Position, std::string, std::unique_ptr<int>)
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();
    const Entity far = allocator.create();
    SparseArray<TestType> array;

    array.erase(far);
    array.insert(entity, makeValue<TestType>(1));
    array.erase(far);

    REQUIRE(array.size() == 1);
    REQUIRE(array.has(entity));
}

TEST_CASE("a component is built in place from arguments", "[ecs][sparse]")
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();
    SparseArray<std::string> array;

    array.emplace(entity, 3, 'x');

    REQUIRE(*array.get(entity) == "xxx");
}

TEST_CASE("values survive the growth of the array", "[ecs][sparse]")
{
    EntityAllocator allocator;
    SparseArray<std::string> array;
    const Entity first = allocator.create();
    array.insert(first, makeValue<std::string>(1));

    for (int i = 0; i < 1000; ++i)
        array.insert(allocator.create(), makeValue<std::string>(i));

    REQUIRE(*array.get(first) == makeValue<std::string>(1));
}

TEST_CASE("the array is addressed by index only", "[ecs][sparse]")
{
    EntityAllocator allocator;
    SparseArray<int> array;
    const Entity old = allocator.create();
    array.insert(old, 7);
    allocator.destroy(old);
    const Entity recycled = allocator.create();

    REQUIRE(recycled.index() == old.index());
    REQUIRE(array.has(recycled));
}

TEST_CASE("a sparse array is a storage and is not copyable", "[ecs][sparse]")
{
    STATIC_REQUIRE(std::is_base_of_v<IStorage, SparseArray<int>>);
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<SparseArray<int>>);
}

TEST_CASE("iterators walk every slot, empty ones included", "[ecs][sparse]")
{
    EntityAllocator allocator;
    SparseArray<int> array;
    const Entity first = allocator.create();
    const Entity second = allocator.create();
    const Entity third = allocator.create();
    array.insert(first, 1);
    array.insert(third, 3);

    REQUIRE(std::distance(array.begin(), array.end()) == 3);
    REQUIRE(array.begin()->value() == 1);
    REQUIRE_FALSE((array.begin() + 1)->has_value());
    REQUIRE((array.begin() + 2)->value() == 3);
    REQUIRE_FALSE(array.has(second));
}

TEST_CASE("an empty array has empty iterator ranges", "[ecs][sparse]")
{
    SparseArray<int> array;

    REQUIRE(array.begin() == array.end());
    REQUIRE(array.cbegin() == array.cend());
}

TEST_CASE("iterators modify components and have const versions",
          "[ecs][sparse]")
{
    EntityAllocator allocator;
    SparseArray<int> array;
    const Entity entity = allocator.create();
    array.insert(entity, 1);

    for (auto &slot : array)
        slot = *slot + 1;
    const SparseArray<int> &view = array;

    REQUIRE(*view.begin() == 2);
    REQUIRE(*view.get(entity) == 2);
    STATIC_REQUIRE(
        std::is_const_v<std::remove_reference_t<decltype(*view.begin())>>);
}
