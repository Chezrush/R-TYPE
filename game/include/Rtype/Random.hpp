/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Random.hpp
*/

#ifndef RANDOM_HPP_
#define RANDOM_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief Small deterministic random number generator (SplitMix64).
 *
 * The same seed gives the same numbers on every machine and standard library,
 * which std::uniform_real_distribution does not guarantee. It never reads the
 * clock or the system, so a game can be replayed from its seed.
 */
class Random {
public:
    explicit Random(std::uint32_t seed = 0) noexcept;

    /** @brief Next 32 random bits. */
    [[nodiscard]] std::uint32_t nextU32() noexcept;

    /** @brief Next number in [min, max), with 24 bits of precision. */
    [[nodiscard]] float nextFloat(float min, float max) noexcept;

    /** @brief Next coin toss. */
    [[nodiscard]] bool nextBool() noexcept;

private:
    std::uint64_t _state;
};

} /* namespace rtype::game */

#endif /* !RANDOM_HPP_ */
