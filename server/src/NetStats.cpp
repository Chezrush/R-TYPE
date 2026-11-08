/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetStats.cpp
*/

#include "NetStats.hpp"

#include <array>
#include <cstdio>

namespace rtype::server {

void NetCounters::recordIn(std::size_t bytes) noexcept
{
    ++datagramsIn;
    bytesIn += bytes;
}

void NetCounters::recordOut(std::size_t bytes) noexcept
{
    ++datagramsOut;
    bytesOut += bytes;
}

void NetCounters::recordDropped() noexcept
{
    ++dropped;
}

NetRates NetRateMeter::sample(const NetCounters &totals, TimePoint now)
{
    NetRates rates;
    const std::chrono::duration<double> elapsed = now - _last;

    if (_started && elapsed.count() > 0.0) {
        rates.bytesInPerSecond =
            static_cast<double>(totals.bytesIn - _lastBytesIn) /
            elapsed.count();
        rates.bytesOutPerSecond =
            static_cast<double>(totals.bytesOut - _lastBytesOut) /
            elapsed.count();
    }
    _started = true;
    _last = now;
    _lastBytesIn = totals.bytesIn;
    _lastBytesOut = totals.bytesOut;
    return rates;
}

std::string formatNetLine(std::size_t clients, const NetRates &rates,
                          std::uint64_t dropped, std::chrono::milliseconds rtt)
{
    std::array<char, 128> text{};

    std::snprintf(text.data(), text.size(),
                  "[net] clients=%zu in=%.1f KB/s out=%.1f KB/s dropped=%llu "
                  "rtt=%lldms",
                  clients, rates.bytesInPerSecond / 1024.0,
                  rates.bytesOutPerSecond / 1024.0,
                  static_cast<unsigned long long>(dropped),
                  static_cast<long long>(rtt.count()));
    return std::string(text.data());
}

} /* namespace rtype::server */
