/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** IStorage.cpp
*/

#include "Engine/Ecs/IStorage.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <set>
#include <type_traits>

#include "Engine/Ecs/EntityAllocator.hpp"

using engine::ecs::Entity;
using engine::ecs::EntityAllocator;
using engine::ecs::IStorage;

namespace {

class SetStorage : public IStorage {
public:
    void insert(Entity entity)
    {
        _entities.insert(entity);
    }

    void erase(Entity entity) override
    {
        _entities.erase(entity);
    }

    [[nodiscard]] bool has(Entity entity) const override
    {
        return _entities.contains(entity);
    }

private:
    std::set<Entity> _entities;
};

} // namespace

TEST_CASE("a storage is used through the interface", "[ecs][storage]")
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();
    auto concrete = std::make_unique<SetStorage>();
    concrete->insert(entity);
    std::unique_ptr<IStorage> storage = std::move(concrete);

    REQUIRE(storage->has(entity));
    storage->erase(entity);
    REQUIRE_FALSE(storage->has(entity));
}

TEST_CASE("erasing an entity without a component does nothing",
          "[ecs][storage]")
{
    EntityAllocator allocator;
    const Entity entity = allocator.create();
    SetStorage storage;

    storage.erase(entity);
    REQUIRE_FALSE(storage.has(entity));
}

TEST_CASE("a storage is abstract and not copyable", "[ecs][storage]")
{
    STATIC_REQUIRE(std::is_abstract_v<IStorage>);
    STATIC_REQUIRE(std::has_virtual_destructor_v<IStorage>);
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<IStorage>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<IStorage>);
}
