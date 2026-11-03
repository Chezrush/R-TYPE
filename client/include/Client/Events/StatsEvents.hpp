/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** StatsEvents.hpp
*/

#ifndef STATSEVENTS_HPP_
#define STATSEVENTS_HPP_

#include <cstdint>
#include <string>

namespace client::events {

/** @brief Network measures, published by the network layer. */
struct NetStatsUpdated {
    float rttMs = 0.0f;
    float lossPercent = 0.0f;
    float bytesInPerSec = 0.0f;
    float bytesOutPerSec = 0.0f;
    std::uint32_t droppedDatagrams = 0;
};

/** @brief A named value to show in the stats overlay (entity count...). */
struct StatsLine {
    std::string name;
    std::string value;
};

} // namespace client::events

#endif /* !STATSEVENTS_HPP_ */
