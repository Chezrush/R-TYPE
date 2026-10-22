/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Registry.tpp
*/

#ifndef REGISTRY_TPP_
#define REGISTRY_TPP_

#include <stdexcept>
#include <utility>

#include "Engine/Ecs/Registry.hpp"

namespace engine::ecs {

template <typename Component>
void Registry::registerComponent()
{
    const std::type_index type(typeid(Component));

    if (!_storages.contains(type))
        _storages.emplace(type, std::make_unique<SparseArray<Component>>());
}

template <typename Component>
SparseArray<Component> &Registry::storage() const
{
    const auto found = _storages.find(std::type_index(typeid(Component)));

    if (found == _storages.end())
        throw std::logic_error("Registry: component type not registered");
    return static_cast<SparseArray<Component> &>(*found->second);
}

template <typename Component>
SparseArray<Component> &Registry::getComponents()
{
    return storage<Component>();
}

template <typename Component>
const SparseArray<Component> &Registry::getComponents() const
{
    return storage<Component>();
}

template <typename Component>
Component *Registry::addComponent(Entity entity, Component component)
{
    return emplaceComponent<Component>(entity, std::move(component));
}

template <typename Component, typename... Args>
Component *Registry::emplaceComponent(Entity entity, Args &&...args)
{
    SparseArray<Component> &array = storage<Component>();

    if (!_entities.isAlive(entity))
        return nullptr;
    return &array.emplace(entity, std::forward<Args>(args)...);
}

template <typename Component>
void Registry::removeComponent(Entity entity)
{
    SparseArray<Component> &array = storage<Component>();

    if (_entities.isAlive(entity))
        array.erase(entity);
}

} // namespace engine::ecs

#endif /* !REGISTRY_TPP_ */
