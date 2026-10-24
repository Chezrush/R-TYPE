/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Series.cpp
*/

#include <algorithm>
#include <numeric>
#include <utility>

#include "Bench.hpp"

namespace bench {

void Series::record(Clock::duration elapsed)
{
    _seconds.push_back(std::chrono::duration<double>(elapsed).count());
}

Result Series::result(std::string name, std::string measured,
                      std::size_t entities, double checksum) const
{
    const double total = std::accumulate(_seconds.begin(), _seconds.end(), 0.0);
    const double best = *std::min_element(_seconds.begin(), _seconds.end());

    return {std::move(name),
            std::move(measured),
            entities,
            _seconds.size(),
            best,
            total / static_cast<double>(_seconds.size()),
            checksum};
}

Clock::duration timed(const std::function<void()> &work)
{
    const auto start = Clock::now();

    work();
    return Clock::now() - start;
}

} // namespace bench
