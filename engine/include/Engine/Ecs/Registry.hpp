/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Registry.hpp
*/

#ifndef REGISTRY_HPP_
#define REGISTRY_HPP_

#include <cstddef>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/EntityAllocator.hpp"
#include "Engine/Ecs/IStorage.hpp"
#include "Engine/Ecs/SparseArray.hpp"

namespace engine::ecs {

/**
 * @brief Single entry point of the ECS: owns the entities and the components.
 *
 * The registry is the only owner of its data, so it cannot be copied.
 *
 * Error policy. Using a component type that was not registered is a
 * programming error: it throws std::logic_error, before the handle is looked
 * at. A stale or unknown entity handle is a normal situation (an entity dies
 * while a late message still designates it): the operation does nothing and
 * says so through its return value, it never throws and never touches the
 * entity that reused the index.
 *
 * Adding a component may reallocate its array: never keep a reference to a
 * component across an add.
 */
class Registry {
public:
    Registry() = default;
    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;
    Registry(Registry &&) noexcept = default;
    Registry &operator=(Registry &&) noexcept = default;
    ~Registry() = default;

    /**
     * @brief Creates a live entity, reusing a freed index when possible.
     * @throws std::length_error when every index is in use.
     */
    [[nodiscard]] Entity spawnEntity();

    /**
     * @brief Destroys an entity and removes all of its components.
     * @return true if the entity was alive, false for a stale or unknown
     * handle (nothing changes in that case).
     */
    bool killEntity(Entity entity) noexcept;

    /** @brief Whether the handle designates an entity that was not killed.
     */
    [[nodiscard]] bool isAlive(Entity entity) const noexcept;

    /** @brief Number of live entities. */
    [[nodiscard]] std::size_t aliveCount() const noexcept;

    /**
     * @brief Handle of the entity using an index, see EntityAllocator.
     * @throws std::out_of_range if the index was never allocated.
     */
    [[nodiscard]] Entity entityAt(std::size_t index) const;

    /**
     * @brief Creates the storage of a component type.
     *
     * Registering a type twice keeps the existing storage and its content.
     */
    template <typename Component>
    void registerComponent();

    /**
     * @brief Storage of a registered component type.
     * @throws std::logic_error if the type was not registered.
     */
    template <typename Component>
    [[nodiscard]] SparseArray<Component> &getComponents();

    template <typename Component>
    [[nodiscard]] const SparseArray<Component> &getComponents() const;

    /**
     * @brief Attaches a component to a live entity, replacing any existing one.
     * @return The component, or nullptr if the handle is stale.
     * @throws std::logic_error if the type was not registered.
     */
    template <typename Component>
    Component *addComponent(Entity entity, Component component);

    /**
     * @brief Builds a component in place on a live entity.
     * @return The component, or nullptr if the handle is stale.
     * @throws std::logic_error if the type was not registered.
     */
    template <typename Component, typename... Args>
    Component *emplaceComponent(Entity entity, Args &&...args);

    /**
     * @brief Removes a component from an entity, if it has one.
     *
     * Does nothing for a stale handle.
     * @throws std::logic_error if the type was not registered.
     */
    template <typename Component>
    void removeComponent(Entity entity);

private:
    using StorageTable =
        std::unordered_map<std::type_index, std::unique_ptr<IStorage>>;

    template <typename Component>
    [[nodiscard]] SparseArray<Component> &storage() const;

    EntityAllocator _entities;
    StorageTable _storages;
};

} // namespace engine::ecs

#include "Engine/Ecs/Registry.tpp"

#endif /* !REGISTRY_HPP_ */
