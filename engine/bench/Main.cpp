/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Main.cpp
*/

#include <charconv>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

#include "Bench.hpp"

namespace {

constexpr std::size_t DEFAULT_ENTITIES = 100000;
constexpr std::size_t DEFAULT_REPETITIONS = 20;
constexpr std::size_t MAX_VALUE = 10000000;

std::optional<std::size_t> parseCount(std::string_view text)
{
    std::size_t value = 0;
    const auto end = text.data() + text.size();
    const auto parsed = std::from_chars(text.data(), end, value);

    if (parsed.ec != std::errc() || parsed.ptr != end || value == 0 ||
        value > MAX_VALUE)
        return std::nullopt;
    return value;
}

std::optional<bench::Config> parseArguments(int argc, char **argv)
{
    bench::Config config{DEFAULT_ENTITIES, DEFAULT_REPETITIONS};

    if (argc > 3)
        return std::nullopt;
    if (argc > 1) {
        const auto entities = parseCount(argv[1]);
        if (!entities)
            return std::nullopt;
        config.entities = *entities;
    }
    if (argc > 2) {
        const auto repetitions = parseCount(argv[2]);
        if (!repetitions)
            return std::nullopt;
        config.repetitions = *repetitions;
    }
    return config;
}

void printHeader()
{
    std::cout << std::left << std::setw(12) << "case" << std::setw(38)
              << "measured" << std::right << std::setw(10) << "entities"
              << std::setw(6) << "reps" << std::setw(12) << "best ms"
              << std::setw(12) << "mean ms" << std::setw(12) << "ns/entity"
              << std::setw(16) << "checksum" << "\n";
}

void printResult(const bench::Result &result)
{
    const double perEntity =
        result.meanSeconds * 1e9 / static_cast<double>(result.entities);

    std::cout << std::left << std::setw(12) << result.name << std::setw(38)
              << result.measured << std::right << std::setw(10)
              << result.entities << std::setw(6) << result.repetitions
              << std::fixed << std::setprecision(3) << std::setw(12)
              << result.bestSeconds * 1e3 << std::setw(12)
              << result.meanSeconds * 1e3 << std::setprecision(1)
              << std::setw(12) << perEntity << std::setw(16)
              << std::setprecision(0) << result.checksum << "\n";
}

} // namespace

int main(int argc, char **argv)
{
    const auto config = parseArguments(argc, argv);

    if (!config) {
        std::cerr << "usage: " << argv[0]
                  << " [entities] [repetitions]  (each 1.." << MAX_VALUE
                  << ")\n";
        return EXIT_FAILURE;
    }
    std::vector<bench::Result> results;
    for (const auto run : {bench::runZipper, bench::runSpawnKill,
                           bench::runCommandBuffer, bench::runScheduler}) {
        const auto part = run(*config);
        results.insert(results.end(), part.begin(), part.end());
    }
    printHeader();
    for (const bench::Result &result : results)
        printResult(result);
    return EXIT_SUCCESS;
}
