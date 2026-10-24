/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameLoop.cpp
*/

#include "Engine/Core/GameLoop.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>

namespace engine::core {

GameLoop::GameLoop(FixedTimestep timestep, Clock clock, StepFunction step)
    : _timestep(timestep),
      _clock(std::move(clock)),
      _step(std::move(step)),
      _last(0.0)
{
    if (!_clock || !_step)
        throw std::invalid_argument("invalid game loop callback");
    _last = _clock();
}

void GameLoop::tick()
{
    const double now = _clock();
    const double elapsed = std::max(0.0, now - _last);
    _last = now;
    const std::size_t due = _timestep.advance(elapsed);

    for (std::size_t i = 0; i < due; ++i)
        _step(_timestep.step());
}

void GameLoop::run()
{
    _last = _clock();
    while (!_stopRequested.load()) {
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    _stopRequested.store(false);
}

void GameLoop::stop() noexcept
{
    _stopRequested.store(true);
}

double GameLoop::alpha() const noexcept
{
    return _timestep.alpha();
}

} // namespace engine::core
