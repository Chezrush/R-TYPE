/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneRoute.hpp
*/

#ifndef SCENEROUTE_HPP_
#define SCENEROUTE_HPP_

#include <cstdint>

#include "Engine/Core/SceneRouter.hpp"

namespace client::scenes {

/** @brief Screens of R-Type. */
enum class SceneRoute : std::uint8_t {
    MAIN_MENU,
    CONNECT,
    GAME,
    SETTINGS,
    CONTROLS,
    HOW_TO_PLAY,
    IN_GAME_MENU,
    RESPAWN,
    GAME_OVER,
    CONSOLE
};

using NavigationRequested = engine::core::NavigationRequested<SceneRoute>;
using OverlayRequested = engine::core::OverlayRequested<SceneRoute>;
using SceneRouter = engine::core::SceneRouter<SceneRoute>;

} // namespace client::scenes

#endif /* !SCENEROUTE_HPP_ */
