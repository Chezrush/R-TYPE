/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Access.hpp
*/

#ifndef ACCESS_HPP_
#define ACCESS_HPP_

#include <set>
#include <typeindex>

namespace engine::ecs {

/**
 * @brief Component types a system reads and writes.
 *
 * The declaration is made by the author of the system and is not checked
 * against what the system really does. It lets the scheduler detect two
 * systems that could not safely run side by side.
 */
class Access {
public:
    using TypeSet = std::set<std::type_index>;

    /** @brief Declares that the system only reads the component type. */
    template <typename Component>
    Access &read();

    /** @brief Declares that the system modifies the component type. */
    template <typename Component>
    Access &write();

    /** @return The component types declared as read. */
    [[nodiscard]] const TypeSet &reads() const noexcept;

    /** @return The component types declared as written. */
    [[nodiscard]] const TypeSet &writes() const noexcept;

    /**
     * @brief Tells whether two systems cannot safely run side by side.
     *
     * True when one writes a type that the other reads or writes. Two reads
     * never conflict, and the relation is symmetric.
     */
    [[nodiscard]] bool conflictsWith(const Access &other) const;

private:
    TypeSet _reads;
    TypeSet _writes;
};

} // namespace engine::ecs

#include "Engine/Ecs/Access.tpp"

#endif /* !ACCESS_HPP_ */
