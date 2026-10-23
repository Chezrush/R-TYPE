/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CommandBuffer.tpp
*/

#ifndef COMMANDBUFFER_TPP_
#define COMMANDBUFFER_TPP_

#include "Engine/Ecs/CommandBuffer.hpp"
#include "Engine/Ecs/Registry.hpp"

namespace engine::ecs {

namespace detail {

template <typename Component>
AddCommand<Component>::AddCommand(Entity entity, Component component)
    : _entity(entity),
      _component(std::move(component))
{}

template <typename Component>
void AddCommand<Component>::apply(Registry &registry)
{
    registry.addComponent<Component>(_entity, std::move(_component));
}

template <typename Component>
RemoveCommand<Component>::RemoveCommand(Entity entity)
    : _entity(entity)
{}

template <typename Component>
void RemoveCommand<Component>::apply(Registry &registry)
{
    registry.removeComponent<Component>(_entity);
}

} // namespace detail

template <typename Component>
void CommandBuffer::add(Entity entity, Component component)
{
    push(std::make_unique<detail::AddCommand<Component>>(entity,
                                                         std::move(component)));
}

template <typename Component, typename... Args>
void CommandBuffer::emplace(Entity entity, Args &&...args)
{
    add<Component>(entity, Component(std::forward<Args>(args)...));
}

template <typename Component>
void CommandBuffer::remove(Entity entity)
{
    push(std::make_unique<detail::RemoveCommand<Component>>(entity));
}

} // namespace engine::ecs

#endif /* !COMMANDBUFFER_TPP_ */
