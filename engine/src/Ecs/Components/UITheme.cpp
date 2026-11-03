/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UITheme.cpp
*/

#include "Engine/Ecs/Components/UITheme.hpp"

namespace engine::ecs::components {

void UITheme::setColor(UIColorRole role, UIColor color) noexcept
{
    if (role != UIColorRole::NONE && role != UIColorRole::COUNT)
        _colors[static_cast<std::size_t>(role)] = color;
}

UIColor UITheme::resolve(UIColorRole role, UIColor fallback) const noexcept
{
    if (role == UIColorRole::COUNT)
        return fallback;
    return _colors[static_cast<std::size_t>(role)].value_or(fallback);
}

UIWidgetStyle UITheme::styled(const UIWidgetStyle &style) const
{
    UIWidgetStyle result = style;

    result.background = resolve(UIColorRole::SURFACE, style.background);
    result.foreground = resolve(UIColorRole::TEXT, style.foreground);
    result.accent = resolve(UIColorRole::ACCENT, style.accent);
    result.focusOutline = resolve(UIColorRole::FOCUS, style.focusOutline);
    return result;
}

bool UITheme::hasPlate(UIColorRole textRole) const noexcept
{
    return _options.textPlates && textRole != UIColorRole::TEXT_ON_PLAYER;
}

UIThemeOptions &UITheme::options() noexcept
{
    return _options;
}

const UIThemeOptions &UITheme::options() const noexcept
{
    return _options;
}

} // namespace engine::ecs::components
