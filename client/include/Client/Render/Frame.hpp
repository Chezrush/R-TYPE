/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Frame.hpp
*/

#ifndef FRAME_HPP_
#define FRAME_HPP_

#include <array>
#include <string>

#include "Client/Scenes/Starfield.hpp"
#include "Client/Settings/Palette.hpp"
#include "Client/World/WorldState.hpp"

namespace client::render {

/**
 * @brief What the window draws in one frame.
 */
struct Frame {
    const world::WorldState &world;
    const scenes::Starfield &stars;
    double now = 0;
    std::string banner;
    int rttMs = 0;
    settings::ColorPalette palette = settings::ColorPalette::NORMAL;
    bool highContrast = false;
    std::array<std::string, 4> cues{};
};

} /* namespace client::render */

#endif /* !FRAME_HPP_ */
