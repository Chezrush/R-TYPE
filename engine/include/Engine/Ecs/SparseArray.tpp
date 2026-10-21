/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SparseArray.tpp
*/

#ifndef SPARSEARRAY_TPP_
#define SPARSEARRAY_TPP_

#include "Engine/Ecs/SparseArray.hpp"

namespace engine::ecs {

template <typename Component>
template <typename... Args>
Component &SparseArray<Component>::emplace(Entity entity, Args &&...args)
{
    const std::size_t index = entity.index();

    if (index >= _slots.size())
        _slots.resize(index + 1);
    return _slots[index].emplace(std::forward<Args>(args)...);
}

template <typename Component>
Component &SparseArray<Component>::insert(Entity entity, Component component)
{
    return emplace(entity, std::move(component));
}

template <typename Component>
void SparseArray<Component>::erase(Entity entity)
{
    if (entity.index() < _slots.size())
        _slots[entity.index()].reset();
}

template <typename Component>
bool SparseArray<Component>::has(Entity entity) const
{
    return get(entity) != nullptr;
}

template <typename Component>
Component *SparseArray<Component>::get(Entity entity) noexcept
{
    if (entity.index() >= _slots.size() || !_slots[entity.index()])
        return nullptr;
    return &*_slots[entity.index()];
}

template <typename Component>
const Component *SparseArray<Component>::get(Entity entity) const noexcept
{
    if (entity.index() >= _slots.size() || !_slots[entity.index()])
        return nullptr;
    return &*_slots[entity.index()];
}

template <typename Component>
std::size_t SparseArray<Component>::size() const noexcept
{
    return _slots.size();
}

template <typename Component>
typename SparseArray<Component>::iterator
SparseArray<Component>::begin() noexcept
{
    return _slots.begin();
}

template <typename Component>
typename SparseArray<Component>::iterator SparseArray<Component>::end() noexcept
{
    return _slots.end();
}

template <typename Component>
typename SparseArray<Component>::const_iterator
SparseArray<Component>::begin() const noexcept
{
    return _slots.begin();
}

template <typename Component>
typename SparseArray<Component>::const_iterator
SparseArray<Component>::end() const noexcept
{
    return _slots.end();
}

template <typename Component>
typename SparseArray<Component>::const_iterator
SparseArray<Component>::cbegin() const noexcept
{
    return _slots.cbegin();
}

template <typename Component>
typename SparseArray<Component>::const_iterator
SparseArray<Component>::cend() const noexcept
{
    return _slots.cend();
}

} // namespace engine::ecs

#endif /* !SPARSEARRAY_TPP_ */
