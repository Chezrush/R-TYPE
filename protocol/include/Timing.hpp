/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Timing.hpp
*/

#ifndef TIMING_HPP_
#define TIMING_HPP_

#include <chrono>

namespace rtype::protocol {

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::microseconds;

} /* namespace rtype::protocol */

#endif /* !TIMING_HPP_ */
