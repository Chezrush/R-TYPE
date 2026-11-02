/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Starfield.cpp
*/

#include "Client/Scenes/Starfield.hpp"

#include <cmath>
#include <random>

namespace client::scenes {

namespace {

constexpr float MIN_SPEED = 0.01f;
constexpr float DEPTH_SPEED = 0.07f;
constexpr float MIN_SIZE = 1.0f;
constexpr float DEPTH_SIZE = 2.0f;
constexpr float MIN_BRIGHTNESS = 90.0f;
constexpr float DEPTH_BRIGHTNESS = 165.0f;

Star makeStar(std::mt19937 &random)
{
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);
    Star star;

    star.position = {unit(random), unit(random)};
    const float depth = unit(random);
    star.speed = MIN_SPEED + depth * DEPTH_SPEED;
    star.size = MIN_SIZE + depth * DEPTH_SIZE;
    star.brightness =
        static_cast<std::uint8_t>(MIN_BRIGHTNESS + depth * DEPTH_BRIGHTNESS);
    return star;
}

} // namespace

Starfield::Starfield(std::size_t count, std::uint32_t seed)
{
    std::mt19937 random(seed);

    _stars.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        _stars.push_back(makeStar(random));
}

void Starfield::advance(float dt)
{
    for (Star &star : _stars) {
        star.position.x -= star.speed * dt;
        star.position.x -= std::floor(star.position.x);
    }
}

const std::vector<Star> &Starfield::stars() const noexcept
{
    return _stars;
}

} // namespace client::scenes
