/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MenuContext.cpp
*/

#include "Client/Scenes/MenuContext.hpp"

#include <utility>

namespace client::scenes {

namespace {

using engine::ecs::components::UIColor;

constexpr UIColor BUTTON_COLOR{24, 36, 90, 255};
constexpr UIColor BUTTON_HOVERED_COLOR{50, 80, 190, 255};
constexpr UIColor BUTTON_PRESSED_COLOR{16, 24, 60, 255};
constexpr UIColor BUTTON_DISABLED_COLOR{40, 40, 50, 180};

} // namespace

engine::ecs::UISceneContext
makeMenuContext(engine::ecs::Registry &registry, engine::core::EventBus &bus,
                engine::ecs::UIInteractionSystem &interaction,
                std::string fontId, const engine::ecs::UITheme *theme)
{
    engine::ecs::UISceneContext context{registry, bus, interaction,
                                        std::move(fontId)};

    context.buttonStyle.normalColor = BUTTON_COLOR;
    context.buttonStyle.hoveredColor = BUTTON_HOVERED_COLOR;
    context.buttonStyle.pressedColor = BUTTON_PRESSED_COLOR;
    context.buttonStyle.disabledColor = BUTTON_DISABLED_COLOR;
    context.theme = theme;
    return context;
}

} // namespace client::scenes
