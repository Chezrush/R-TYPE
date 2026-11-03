/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIComponents.cpp
*/

#include "Engine/Ecs/Components/UIComponents.hpp"

#include "Engine/Ecs/Components/UIShape.hpp"

namespace engine::ecs {

void registerUIComponents(Registry &registry)
{
    registry.registerComponent<components::UITransform>();
    registry.registerComponent<components::UIButton>();
    registry.registerComponent<components::UIPanel>();
    registry.registerComponent<components::UIProgressBar>();
    registry.registerComponent<components::UIText>();
    registry.registerComponent<components::UITextInput>();
    registry.registerComponent<components::UISlider>();
    registry.registerComponent<components::UIToggle>();
    registry.registerComponent<components::UIDropdown>();
    registry.registerComponent<components::UIScrollList>();
    registry.registerComponent<components::UIShape>();
}

} // namespace engine::ecs
