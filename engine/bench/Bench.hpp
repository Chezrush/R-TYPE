/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Bench.hpp
*/

#ifndef BENCH_HPP_
#define BENCH_HPP_

#include <chrono>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace bench {

using Clock = std::chrono::steady_clock;

struct Config {
    std::size_t entities;
    std::size_t repetitions;
};

struct Result {
    std::string name;
    std::string measured;
    std::size_t entities;
    std::size_t repetitions;
    double bestSeconds;
    double meanSeconds;
    double checksum;
};

/** Accumulates the duration of each repetition of one case. */
class Series {
public:
    void record(Clock::duration elapsed);

    [[nodiscard]] Result result(std::string name, std::string measured,
                                std::size_t entities, double checksum) const;

private:
    std::vector<double> _seconds;
};

/** Runs the work once and returns how long it took. */
[[nodiscard]] Clock::duration timed(const std::function<void()> &work);

[[nodiscard]] std::vector<Result> runZipper(const Config &config);
[[nodiscard]] std::vector<Result> runSpawnKill(const Config &config);
[[nodiscard]] std::vector<Result> runCommandBuffer(const Config &config);
[[nodiscard]] std::vector<Result> runScheduler(const Config &config);

} // namespace bench

#endif /* !BENCH_HPP_ */
