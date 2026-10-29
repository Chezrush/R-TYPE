/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Scheduler.tpp
*/

#ifndef SCHEDULER_TPP_
#define SCHEDULER_TPP_

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Engine/Ecs/IndexedZipper.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"

namespace engine::ecs {

namespace detail {

template <typename Component>
void declareAccess(Access &access)
{
    if constexpr (std::is_const_v<Component>)
        access.read<std::remove_const_t<Component>>();
    else
        access.write<Component>();
}

template <typename... Components>
Access makeAccess()
{
    Access access;

    (declareAccess<Components>(access), ...);
    return access;
}

template <typename Component>
decltype(auto) arrayOf(Registry &registry)
{
    using Plain = std::remove_const_t<Component>;

    if constexpr (std::is_const_v<Component>)
        return std::as_const(registry).template getComponents<Plain>();
    else
        return registry.template getComponents<Plain>();
}

template <typename... Components, typename Function>
void runTyped(Registry &registry, CommandBuffer &commands, double deltaTime,
              Function &function)
{
    for (auto row : makeIndexedZipper(arrayOf<Components>(registry)...)) {
        std::apply(
            [&](std::size_t index, auto &...components) {
                function(registry.entityAt(index), components..., commands,
                         deltaTime);
            },
            row);
    }
}

} // namespace detail

template <typename... Components, typename Function>
Scheduler::SystemId Scheduler::addSystem(std::string name, Phase phase,
                                         Function &&function)
{
    return addSystem(std::move(name), phase,
                     detail::makeAccess<Components...>(),
                     [function = std::forward<Function>(function)](
                         Registry &registry, CommandBuffer &commands,
                         double deltaTime) mutable {
                         detail::runTyped<Components...>(registry, commands,
                                                         deltaTime, function);
                     });
}

} // namespace engine::ecs

#endif /* !SCHEDULER_TPP_ */
