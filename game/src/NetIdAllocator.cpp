/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetIdAllocator.cpp
*/

#include "Rtype/NetIdAllocator.hpp"

namespace rtype::game {

std::optional<std::uint16_t> NetIdAllocator::allocate()
{
    std::uint16_t id = 0;

    if (_nextFresh < CAPACITY) {
        id = static_cast<std::uint16_t>(_nextFresh++);
    } else if (!_released.empty()) {
        id = _released.front();
        _released.pop_front();
    } else {
        return std::nullopt;
    }
    _inUse[id] = true;
    ++_alive;
    return id;
}

bool NetIdAllocator::release(std::uint16_t id)
{
    if (!_inUse[id]) {
        return false;
    }
    _inUse[id] = false;
    _released.push_back(id);
    --_alive;
    return true;
}

bool NetIdAllocator::isAllocated(std::uint16_t id) const noexcept
{
    return _inUse[id];
}

std::size_t NetIdAllocator::aliveCount() const noexcept
{
    return _alive;
}

} /* namespace rtype::game */
