/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** FixedTimestep.hpp
*/

#ifndef FIXEDTIMESTEP_HPP_
#define FIXEDTIMESTEP_HPP_

#include <cstddef>

namespace engine::core {

/**
 * @brief Turns elapsed real time into a number of fixed simulation steps.
 *
 * Pure accumulator: it reads no clock, the caller passes the elapsed time in
 * seconds. The remainder smaller than one step is kept for the next call.
 * Whole steps beyond the per-call maximum are dropped so that a long stall
 * cannot trigger an unbounded catch-up. Not thread-safe.
 */
class FixedTimestep {
public:
    /**
     * @param step Duration of one simulation step, in seconds.
     * @param maxSteps Maximum number of steps returned by one advance().
     * @throws std::invalid_argument if the step is not a positive finite
     * number or if maxSteps is zero.
     */
    FixedTimestep(double step, std::size_t maxSteps);

    /**
     * @brief Adds elapsed time and consumes the steps that are due.
     * @param elapsed Real time since the previous call, in seconds.
     * @return The number of steps to simulate, at most the configured
     * maximum.
     * @throws std::invalid_argument if elapsed is negative or not finite.
     */
    [[nodiscard]] std::size_t advance(double elapsed);

    /** @return The leftover time as a fraction of a step, in [0, 1). */
    [[nodiscard]] double alpha() const noexcept;

    /** @return The duration of one step, in seconds. */
    [[nodiscard]] double step() const noexcept;

    /** @return The maximum number of steps per advance(). */
    [[nodiscard]] std::size_t maxSteps() const noexcept;

private:
    double _step;
    std::size_t _maxSteps;
    double _accumulator = 0.0;
};

} // namespace engine::core

#endif /* !FIXEDTIMESTEP_HPP_ */
