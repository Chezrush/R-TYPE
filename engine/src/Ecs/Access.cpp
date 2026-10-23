/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Access.cpp
*/

#include "Engine/Ecs/Access.hpp"

#include <algorithm>

namespace engine::ecs {

namespace {

bool intersects(const Access::TypeSet &first, const Access::TypeSet &second)
{
    return std::any_of(first.begin(), first.end(),
                       [&second](const std::type_index &type) {
                           return second.contains(type);
                       });
}

} // namespace

const Access::TypeSet &Access::reads() const noexcept
{
    return _reads;
}

const Access::TypeSet &Access::writes() const noexcept
{
    return _writes;
}

bool Access::conflictsWith(const Access &other) const
{
    return intersects(_writes, other._writes) ||
           intersects(_writes, other._reads) ||
           intersects(_reads, other._writes);
}

} // namespace engine::ecs
