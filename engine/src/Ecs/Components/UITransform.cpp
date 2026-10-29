/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UITransform.cpp
*/

#include "Engine/Ecs/Components/UITransform.hpp"

namespace engine::ecs::components {

std::strong_ordering operator<=>(const UITransform &lhs,
                                 const UITransform &rhs) noexcept
{
    return lhs.zIndex <=> rhs.zIndex;
}

} // namespace engine::ecs::components
