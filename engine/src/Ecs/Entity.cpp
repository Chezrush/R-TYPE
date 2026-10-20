/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Entity.cpp
*/

#include "Engine/Ecs/Entity.hpp"

namespace engine::ecs {

Entity::Entity(std::uint32_t index, std::uint32_t generation) noexcept
    : _index(index),
      _generation(generation)
{}

std::uint32_t Entity::index() const noexcept
{
    return _index;
}

std::uint32_t Entity::generation() const noexcept
{
    return _generation;
}

Entity::operator std::size_t() const noexcept
{
    return _index;
}

} // namespace engine::ecs
