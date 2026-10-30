/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** BeamGauge.cpp
*/

#include "Client/Hud/BeamGauge.hpp"

#include <algorithm>

namespace client::hud {

BeamGauge::BeamGauge(float chargeSeconds) noexcept
    : _chargeSeconds(chargeSeconds)
{}

void BeamGauge::update(float dt, bool shootHeld) noexcept
{
    if (!shootHeld) {
        reset();
        return;
    }
    if (_chargeSeconds <= 0.0f) {
        _charge = 1.0f;
        return;
    }
    _charge = std::min(1.0f, _charge + std::max(0.0f, dt) / _chargeSeconds);
}

void BeamGauge::reset() noexcept
{
    _charge = 0.0f;
}

float BeamGauge::charge() const noexcept
{
    return _charge;
}

bool BeamGauge::isFull() const noexcept
{
    return _charge >= 1.0f;
}

} // namespace client::hud
