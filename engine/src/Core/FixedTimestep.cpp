/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** FixedTimestep.cpp
*/

#include "Engine/Core/FixedTimestep.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace engine::core {

FixedTimestep::FixedTimestep(double step, std::size_t maxSteps)
    : _step(step),
      _maxSteps(maxSteps)
{
    if (!std::isfinite(step) || step <= 0.0)
        throw std::invalid_argument("invalid step");
    if (maxSteps == 0)
        throw std::invalid_argument("invalid maximum of steps");
}

std::size_t FixedTimestep::advance(double elapsed)
{
    if (!std::isfinite(elapsed) || elapsed < 0.0)
        throw std::invalid_argument("invalid elapsed time");
    _accumulator += elapsed;
    const double whole = std::floor(_accumulator / _step);
    _accumulator = std::max(0.0, _accumulator - whole * _step);
    if (_accumulator >= _step)
        _accumulator = 0.0;
    if (whole >= static_cast<double>(_maxSteps))
        return _maxSteps;
    return static_cast<std::size_t>(whole);
}

double FixedTimestep::alpha() const noexcept
{
    return _accumulator / _step;
}

double FixedTimestep::step() const noexcept
{
    return _step;
}

std::size_t FixedTimestep::maxSteps() const noexcept
{
    return _maxSteps;
}

} // namespace engine::core
