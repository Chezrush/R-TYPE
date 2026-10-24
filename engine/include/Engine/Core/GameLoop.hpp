/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameLoop.hpp
*/

#ifndef GAMELOOP_HPP_
#define GAMELOOP_HPP_

#include <atomic>
#include <functional>

#include "Engine/Core/FixedTimestep.hpp"

namespace engine::core {

/**
 * @brief Calls a step callback at a fixed rate, whatever the frame rate.
 *
 * Time comes from an injected clock, so tests drive the loop without real
 * time. The loop knows nothing about what the step does: it is the callback
 * that runs the scheduler, never the other way round.
 */
class GameLoop {
public:
    /** @brief Monotonic time in seconds. */
    using Clock = std::function<double()>;

    /** @brief Called once per fixed step with the step duration in seconds. */
    using StepFunction = std::function<void(double)>;

    /**
     * @param timestep Fixed step duration and per-iteration step cap.
     * @param clock Source of time, read once per iteration.
     * @param step Simulation step.
     * @throws std::invalid_argument if the clock or the step is empty.
     */
    GameLoop(FixedTimestep timestep, Clock clock, StepFunction step);

    /**
     * @brief Runs one iteration: reads the clock, then calls the step
     * callback once for each fixed step that is due.
     *
     * A clock that goes backwards counts as no elapsed time. An exception
     * thrown by the callback reaches the caller.
     */
    void tick();

    /**
     * @brief Repeats tick() until stop() is called, sleeping briefly
     * between iterations so that the thread does not spin.
     *
     * A stop() requested before run() makes it return at once. The request
     * is consumed on return, so the loop can run again.
     */
    void run();

    /** @brief Asks run() to return. Safe to call from any thread. */
    void stop() noexcept;

    /** @return The leftover time as a fraction of a step, for interpolation.
     */
    [[nodiscard]] double alpha() const noexcept;

private:
    FixedTimestep _timestep;
    Clock _clock;
    StepFunction _step;
    double _last;
    std::atomic<bool> _stopRequested = false;
};

} // namespace engine::core

#endif /* !GAMELOOP_HPP_ */
