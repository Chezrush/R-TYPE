/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Random.cpp
*/

#include "Rtype/Random.hpp"

namespace rtype::game {

namespace {

constexpr std::uint64_t INCREMENT = 0x9E3779B97F4A7C15ULL;
constexpr std::uint64_t MULTIPLY_A = 0xBF58476D1CE4E5B9ULL;
constexpr std::uint64_t MULTIPLY_B = 0x94D049BB133111EBULL;
constexpr float UNIT_STEPS = 16777216.0F;

} // namespace

Random::Random(std::uint32_t seed) noexcept
    : _state(seed)
{}

std::uint32_t Random::nextU32() noexcept
{
    _state += INCREMENT;
    std::uint64_t value = _state;

    value = (value ^ (value >> 30U)) * MULTIPLY_A;
    value = (value ^ (value >> 27U)) * MULTIPLY_B;
    value ^= value >> 31U;
    return static_cast<std::uint32_t>(value >> 32U);
}

float Random::nextFloat(float min, float max) noexcept
{
    const float unit = static_cast<float>(nextU32() >> 8U) / UNIT_STEPS;

    return min + (max - min) * unit;
}

bool Random::nextBool() noexcept
{
    return (nextU32() >> 31U) != 0;
}

} /* namespace rtype::game */
