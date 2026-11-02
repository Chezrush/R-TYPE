/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIOverlay.hpp
*/

#ifndef UIOVERLAY_HPP_
#define UIOVERLAY_HPP_

#include "Engine/Ecs/Components/UIColor.hpp"
#include "Engine/Ecs/UIScene.hpp"

namespace engine::ecs {

/** @brief Added to the z-index of an overlay's widgets, to stay on top. */
inline constexpr int UI_OVERLAY_Z_INDEX = 1000;

/**
 * @brief UI scene drawn above the scene below it (menu, modal message...).
 *
 * The input only reaches the overlay. By default the scene below keeps
 * updating, as a game that goes on behind a menu; pass false to freeze it.
 */
class UIOverlay : public UIScene {
public:
    explicit UIOverlay(UISceneContext context, bool updatesBelow = true);

    [[nodiscard]] bool isOverlay() const noexcept override;
    [[nodiscard]] bool updatesSceneBelow() const noexcept override;

protected:
    /** @brief Covers the whole window, behind the overlay's other widgets. */
    Entity addBackdrop(components::UIColor color);

private:
    bool _updatesBelow;
};

} // namespace engine::ecs

#endif /* !UIOVERLAY_HPP_ */
