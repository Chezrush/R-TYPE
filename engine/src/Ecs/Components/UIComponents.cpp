/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIComponents.cpp
*/

#include "Engine/Ecs/Components/UIComponents.hpp"

namespace engine::ecs {

void registerUIComponents(Registry &registry)
{
    registry.registerComponent<components::UITransform>();
    registry.registerComponent<components::UIButton>();
    registry.registerComponent<components::UIPanel>();
    registry.registerComponent<components::UIProgressBar>();
    registry.registerComponent<components::UIText>();
}

} // namespace engine::ecs
