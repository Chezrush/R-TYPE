/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SparseArray.hpp
*/

#ifndef SPARSEARRAY_HPP_
#define SPARSEARRAY_HPP_

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/IStorage.hpp"

namespace engine::ecs {

/**
 * @brief Storage of one component type, addressed by entity index.
 *
 * Slot i holds the component of the entity whose index is i, or nothing. The
 * array only looks at Entity::index(): it does not know whether a handle is
 * stale, that check belongs to the registry. Growing the array invalidates
 * every pointer and reference previously returned.
 *
 * @tparam Component Any move-constructible type.
 */
template <typename Component>
class SparseArray : public IStorage {
public:
    using Slot = std::optional<Component>;
    using Container = std::vector<Slot>;
    using Iterator = typename Container::iterator;
    using ConstIterator = typename Container::const_iterator;

    /**
     * @brief Builds the component of an entity in place.
     *
     * An existing component of the entity is replaced.
     * @return The new component.
     */
    template <typename... Args>
    Component &emplace(Entity entity, Args &&...args);

    /** @brief Stores a component for an entity, replacing any existing one. */
    Component &insert(Entity entity, Component component);

    void erase(Entity entity) override;

    [[nodiscard]] bool has(Entity entity) const override;

    /** @return The component of the entity, or nullptr if it has none. */
    [[nodiscard]] Component *get(Entity entity) noexcept;

    /** @return The component of the entity, or nullptr if it has none. */
    [[nodiscard]] const Component *get(Entity entity) const noexcept;

    /** @brief Number of slots, one past the highest index ever stored. */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Iterators over every slot, empty ones included.
     *
     * Skipping the entities without a component is left to the zipper.
     * Emplacing or inserting may reallocate and invalidate them.
     */
    [[nodiscard]] Iterator begin() noexcept;
    [[nodiscard]] Iterator end() noexcept;
    [[nodiscard]] ConstIterator begin() const noexcept;
    [[nodiscard]] ConstIterator end() const noexcept;
    [[nodiscard]] ConstIterator cbegin() const noexcept;
    [[nodiscard]] ConstIterator cend() const noexcept;

private:
    Container _slots;
};

} // namespace engine::ecs

#include "Engine/Ecs/SparseArray.tpp"

#endif /* !SPARSEARRAY_HPP_ */
