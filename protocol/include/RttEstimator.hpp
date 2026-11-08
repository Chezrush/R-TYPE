/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RttEstimator.hpp
*/

#ifndef RTTESTIMATOR_HPP_
#define RTTESTIMATOR_HPP_

#include "Timing.hpp"

namespace rtype::protocol {

/**
 * @brief Smoothed round trip time, srtt += (rtt - srtt) / 8.
 */
class RttEstimator {
public:
    void addSample(Duration rtt) noexcept;

    [[nodiscard]] bool hasSample() const noexcept;
    [[nodiscard]] Duration smoothed() const noexcept;

    /**
     * @brief Delay before a datagram is considered lost: 1 s before
     * the first sample, then 2 srtt clamped to [100 ms, 3 s].
     */
    [[nodiscard]] Duration retransmissionTimeout() const noexcept;

private:
    bool _hasSample = false;
    Duration _srtt{0};
};

} /* namespace rtype::protocol */

#endif /* !RTTESTIMATOR_HPP_ */
