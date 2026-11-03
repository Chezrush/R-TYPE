/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** StatsModel.cpp
*/

#include "Client/Stats/StatsModel.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace client::stats {

namespace {

constexpr float MS_PER_SECOND = 1000.0f;
constexpr float KILO = 1024.0f;

std::string fixed(float value, int decimals)
{
    std::ostringstream out;

    out << std::fixed << std::setprecision(decimals) << value;
    return out.str();
}

} // namespace

RollingSeries::RollingSeries(std::size_t capacity)
    : _ring(std::max<std::size_t>(capacity, 1), 0.0f)
{}

void RollingSeries::push(float value)
{
    _ring[_next] = value;
    _next = (_next + 1) % _ring.size();
    _size = std::min(_size + 1, _ring.size());
}

std::vector<float> RollingSeries::values() const
{
    std::vector<float> ordered;
    const std::size_t first = (_next + _ring.size() - _size) % _ring.size();

    ordered.reserve(_size);
    for (std::size_t i = 0; i < _size; ++i)
        ordered.push_back(_ring[(first + i) % _ring.size()]);
    return ordered;
}

std::size_t RollingSeries::capacity() const noexcept
{
    return _ring.size();
}

std::size_t RollingSeries::size() const noexcept
{
    return _size;
}

void FrameMeter::addFrame(float dt) noexcept
{
    _frameMs = dt * MS_PER_SECOND;
    _elapsed += dt;
    ++_frames;
    if (_elapsed < WINDOW_SECONDS)
        return;
    _fps = static_cast<float>(_frames) / _elapsed;
    _elapsed = 0.0f;
    _frames = 0;
}

float FrameMeter::fps() const noexcept
{
    return _fps;
}

float FrameMeter::frameMs() const noexcept
{
    return _frameMs;
}

void StatsModel::addFrame(float dt)
{
    _meter.addFrame(dt);
    _frameTimes.push(dt * MS_PER_SECOND);
}

void StatsModel::setNetStats(const events::NetStatsUpdated &stats)
{
    _hasNetStats = true;
    _net = stats;
    _rtts.push(stats.rttMs);
    _losses.push(stats.lossPercent);
}

void StatsModel::setLine(const std::string &name, const std::string &value)
{
    const auto found =
        std::find_if(_lines.begin(), _lines.end(),
                     [&name](const auto &line) { return line.first == name; });

    if (found != _lines.end())
        found->second = value;
    else if (_lines.size() < MAX_EXTRA_LINES)
        _lines.emplace_back(name, value);
}

std::string StatsModel::frameText() const
{
    return "FPS " + fixed(_meter.fps(), 0) + "   frame " +
           fixed(_meter.frameMs(), 1) + " ms";
}

std::string StatsModel::rttText() const
{
    return _hasNetStats ? "RTT " + fixed(_net.rttMs, 0) + " ms"
                        : "RTT: no data";
}

std::string StatsModel::lossText() const
{
    return _hasNetStats ? "Loss " + fixed(_net.lossPercent, 1) + " %"
                        : "Loss: no data";
}

std::string StatsModel::trafficText() const
{
    if (!_hasNetStats)
        return "Traffic: no data";
    return "In " + formatRate(_net.bytesInPerSec) + "   out " +
           formatRate(_net.bytesOutPerSec) + "   dropped " +
           std::to_string(_net.droppedDatagrams);
}

std::vector<std::string> StatsModel::extraLines() const
{
    std::vector<std::string> lines;

    lines.reserve(_lines.size());
    for (const auto &[name, value] : _lines)
        lines.push_back(std::string(name).append(": ").append(value));
    return lines;
}

const RollingSeries &StatsModel::frameTimes() const noexcept
{
    return _frameTimes;
}

const RollingSeries &StatsModel::rtts() const noexcept
{
    return _rtts;
}

const RollingSeries &StatsModel::losses() const noexcept
{
    return _losses;
}

std::string StatsModel::formatRate(float bytesPerSecond)
{
    if (bytesPerSecond < KILO)
        return fixed(bytesPerSecond, 0) + " B/s";
    if (bytesPerSecond < KILO * KILO)
        return fixed(bytesPerSecond / KILO, 1) + " KB/s";
    return fixed(bytesPerSecond / (KILO * KILO), 1) + " MB/s";
}

} // namespace client::stats
