/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Starfield.hpp
*/

#ifndef STARFIELD_HPP_
#define STARFIELD_HPP_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Engine/Core/Vec2.hpp"

namespace client::scenes {

/** @brief One star; x and y are fractions of the window size (0 to 1). */
struct Star {
    engine::core::Vec2 position;
    float speed = 0.0f;
    float size = 1.0f;
    std::uint8_t brightness = 255;
};

/**
 * @brief Stars scrolling to the left, wrapping around. No drawing here.
 *
 * The same seed gives the same stars. Bigger stars look closer: they are
 * brighter and move faster.
 */
class Starfield {
public:
    Starfield(std::size_t count, std::uint32_t seed);

    /** @brief Moves the stars by a time step in seconds. */
    void advance(float dt);

    [[nodiscard]] const std::vector<Star> &stars() const noexcept;

private:
    std::vector<Star> _stars;
};

} // namespace client::scenes

#endif /* !STARFIELD_HPP_ */
