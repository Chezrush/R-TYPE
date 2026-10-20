/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EntityAllocator.cpp
*/

#include "Engine/Ecs/EntityAllocator.hpp"

#include <limits>
#include <stdexcept>

namespace engine::ecs {

Entity EntityAllocator::create()
{
    if (!_freeIndices.empty()) {
        const std::uint32_t index = _freeIndices.back();
        _freeIndices.pop_back();
        return Entity(index, _generations[index]);
    }
    if (_generations.size() >= std::numeric_limits<std::uint32_t>::max())
        throw std::length_error("EntityAllocator: no free index left");
    _generations.push_back(0);
    return Entity(static_cast<std::uint32_t>(_generations.size() - 1), 0);
}

bool EntityAllocator::destroy(Entity entity) noexcept
{
    if (!isAlive(entity))
        return false;
    _generations[entity.index()] += 1;
    _freeIndices.push_back(entity.index());
    return true;
}

bool EntityAllocator::isAlive(Entity entity) const noexcept
{
    if (entity.index() >= _generations.size())
        return false;
    return _generations[entity.index()] == entity.generation();
}

std::size_t EntityAllocator::aliveCount() const noexcept
{
    return _generations.size() - _freeIndices.size();
}

} // namespace engine::ecs
