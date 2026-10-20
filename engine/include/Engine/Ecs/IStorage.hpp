/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** IStorage.hpp
*/

#ifndef ISTORAGE_HPP_
#define ISTORAGE_HPP_

#include "Engine/Ecs/Entity.hpp"

namespace engine::ecs {

/**
 * @brief Type-erased view of a component array.
 *
 * The registry holds one storage per component type through this interface,
 * so it can remove an entity from every array without knowing their types.
 * Storages are owned through std::unique_ptr and are not copyable.
 */
class IStorage {
public:
    IStorage() = default;
    virtual ~IStorage() = default;

    IStorage(const IStorage &) = delete;
    IStorage &operator=(const IStorage &) = delete;
    IStorage(IStorage &&) = delete;
    IStorage &operator=(IStorage &&) = delete;

    /**
     * @brief Removes the component of an entity, if it has one.
     *
     * Erasing an entity without a component is a no-op.
     */
    virtual void erase(Entity entity) = 0;

    /** @brief Whether the entity has a component in this storage. */
    [[nodiscard]] virtual bool has(Entity entity) const = 0;
};

} // namespace engine::ecs

#endif /* !ISTORAGE_HPP_ */
