/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetStats.hpp
*/

#ifndef NETSTATS_HPP_
#define NETSTATS_HPP_

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

namespace rtype::server {

/**
 * @brief Traffic counters, since the start of a connection or of the server.
 */
struct NetCounters {
    std::uint64_t datagramsIn = 0;
    std::uint64_t bytesIn = 0;
    std::uint64_t datagramsOut = 0;
    std::uint64_t bytesOut = 0;
    std::uint64_t dropped = 0;

    void recordIn(std::size_t bytes) noexcept;
    void recordOut(std::size_t bytes) noexcept;
    void recordDropped() noexcept;
};

/**
 * @brief Bytes per second over the last sampled window.
 */
struct NetRates {
    double bytesInPerSecond = 0.0;
    double bytesOutPerSecond = 0.0;
};

/**
 * @brief Turns growing counters into rates.
 *
 * Each call to sample() measures the traffic since the previous call.
 * It holds no clock: the caller gives the time, so it is testable.
 */
class NetRateMeter {
public:
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @brief Returns the rates since the last sample.
     *
     * The first call, or a call with no elapsed time, returns zeros.
     */
    [[nodiscard]] NetRates sample(const NetCounters &totals, TimePoint now);

private:
    bool _started = false;
    TimePoint _last;
    std::uint64_t _lastBytesIn = 0;
    std::uint64_t _lastBytesOut = 0;
};

/**
 * @brief Formats the periodic server line, for example
 * "[net] clients=2 in=1.4 KB/s out=4.5 KB/s dropped=0 rtt=3ms".
 */
[[nodiscard]] std::string formatNetLine(std::size_t clients,
                                        const NetRates &rates,
                                        std::uint64_t dropped,
                                        std::chrono::milliseconds rtt);

} /* namespace rtype::server */

#endif /* !NETSTATS_HPP_ */
