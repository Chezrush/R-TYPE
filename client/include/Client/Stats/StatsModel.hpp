/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** StatsModel.hpp
*/

#ifndef STATSMODEL_HPP_
#define STATSMODEL_HPP_

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "Client/Events/StatsEvents.hpp"

namespace client::stats {

/** @brief The last values of a measure, oldest first. */
class RollingSeries {
public:
    explicit RollingSeries(std::size_t capacity);

    void push(float value);

    /** @brief The samples, oldest first. */
    [[nodiscard]] std::vector<float> values() const;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<float> _ring;
    std::size_t _next = 0;
    std::size_t _size = 0;
};

/**
 * @brief Frames per second over the last half second, and the last frame time.
 */
class FrameMeter {
public:
    static constexpr float WINDOW_SECONDS = 0.5f;

    void addFrame(float dt) noexcept;

    [[nodiscard]] float fps() const noexcept;
    [[nodiscard]] float frameMs() const noexcept;

private:
    float _elapsed = 0.0f;
    int _frames = 0;
    float _fps = 0.0f;
    float _frameMs = 0.0f;
};

/** @brief Everything the stats overlay shows, with no UI. */
class StatsModel {
public:
    static constexpr std::size_t SAMPLES = 120;
    static constexpr std::size_t MAX_EXTRA_LINES = 8;

    void addFrame(float dt);
    void setNetStats(const events::NetStatsUpdated &stats);

    /** @brief Adds or replaces a line; past MAX_EXTRA_LINES, new names are left
     * out. */
    void setLine(const std::string &name, const std::string &value);

    [[nodiscard]] std::string frameText() const;
    [[nodiscard]] std::string rttText() const;
    [[nodiscard]] std::string lossText() const;
    [[nodiscard]] std::string trafficText() const;
    [[nodiscard]] std::vector<std::string> extraLines() const;

    [[nodiscard]] const RollingSeries &frameTimes() const noexcept;
    [[nodiscard]] const RollingSeries &rtts() const noexcept;
    [[nodiscard]] const RollingSeries &losses() const noexcept;

    /** @brief "512 B/s", "3.4 KB/s", "1.2 MB/s" */
    [[nodiscard]] static std::string formatRate(float bytesPerSecond);

private:
    FrameMeter _meter;
    RollingSeries _frameTimes{SAMPLES};
    RollingSeries _rtts{SAMPLES};
    RollingSeries _losses{SAMPLES};
    bool _hasNetStats = false;
    events::NetStatsUpdated _net;
    std::vector<std::pair<std::string, std::string>> _lines;
};

} // namespace client::stats

#endif /* !STATSMODEL_HPP_ */
