/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RttEstimator.cpp
*/

#include "RttEstimator.hpp"

#include <algorithm>
#include <chrono>

namespace rtype::protocol {

namespace {

constexpr Duration MIN_TIMEOUT = std::chrono::milliseconds(100);
constexpr Duration MAX_TIMEOUT = std::chrono::seconds(3);
constexpr Duration INITIAL_TIMEOUT = std::chrono::seconds(1);

} // namespace

void RttEstimator::addSample(Duration rtt) noexcept
{
    if (rtt < Duration::zero()) {
        return;
    }
    if (!_hasSample) {
        _hasSample = true;
        _srtt = rtt;
        return;
    }
    _srtt += (rtt - _srtt) / 8;
}

bool RttEstimator::hasSample() const noexcept
{
    return _hasSample;
}

Duration RttEstimator::smoothed() const noexcept
{
    return _srtt;
}

Duration RttEstimator::retransmissionTimeout() const noexcept
{
    if (!_hasSample) {
        return INITIAL_TIMEOUT;
    }
    return std::clamp(_srtt * 2, MIN_TIMEOUT, MAX_TIMEOUT);
}

} /* namespace rtype::protocol */
