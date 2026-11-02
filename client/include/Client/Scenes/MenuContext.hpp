/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MenuContext.hpp
*/

#ifndef MENUCONTEXT_HPP_
#define MENUCONTEXT_HPP_

#include <string>

#include "Engine/Ecs/UIScene.hpp"

namespace client::scenes {

/** @brief Scene context with the look of the R-Type menus. */
[[nodiscard]] engine::ecs::UISceneContext
makeMenuContext(engine::ecs::Registry &registry, engine::core::EventBus &bus,
                engine::ecs::UIInteractionSystem &interaction,
                std::string fontId);

} // namespace client::scenes

#endif /* !MENUCONTEXT_HPP_ */
