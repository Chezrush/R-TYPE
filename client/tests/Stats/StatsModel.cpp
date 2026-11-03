/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** StatsModel.cpp
*/

#include "Client/Stats/StatsModel.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>

using client::events::NetStatsUpdated;
using client::stats::FrameMeter;
using client::stats::RollingSeries;
using client::stats::StatsModel;

TEST_CASE("a series keeps its last values, oldest first", "[client][stats]")
{
    RollingSeries series(3);

    REQUIRE(series.values().empty());
    for (const float value : {1.0f, 2.0f, 3.0f, 4.0f, 5.0f})
        series.push(value);

    REQUIRE(series.size() == 3);
    REQUIRE(series.values() == std::vector<float>{3.0f, 4.0f, 5.0f});
}

TEST_CASE("fps is counted over half a second", "[client][stats]")
{
    FrameMeter meter;

    for (int i = 0; i < 29; ++i)
        meter.addFrame(1.0f / 60.0f);
    REQUIRE(meter.fps() == 0.0f);
    meter.addFrame(1.0f / 60.0f);

    REQUIRE(meter.fps() > 59.0f);
    REQUIRE(meter.fps() < 61.0f);
    REQUIRE(meter.frameMs() > 16.6f);
    REQUIRE(meter.frameMs() < 16.7f);
}

TEST_CASE("the frame line shows fps and frame time", "[client][stats]")
{
    StatsModel model;

    for (int i = 0; i < 30; ++i)
        model.addFrame(0.02f);

    REQUIRE(model.frameText() == "FPS 50   frame 20.0 ms");
    REQUIRE(model.frameTimes().size() == 30);
    REQUIRE(model.frameTimes().values().back() == 20.0f);
}

TEST_CASE("network lines say when there is no data yet", "[client][stats]")
{
    const StatsModel model;

    REQUIRE(model.rttText() == "RTT: no data");
    REQUIRE(model.lossText() == "Loss: no data");
    REQUIRE(model.trafficText() == "Traffic: no data");
    REQUIRE(model.rtts().size() == 0);
}

TEST_CASE("network values are shown and graphed", "[client][stats]")
{
    StatsModel model;

    model.setNetStats(NetStatsUpdated{42.4f, 1.25f, 3500.0f, 700.0f, 3});
    model.setNetStats(NetStatsUpdated{61.0f, 0.0f, 3500.0f, 700.0f, 4});

    REQUIRE(model.rttText() == "RTT 61 ms");
    REQUIRE(model.lossText() == "Loss 0.0 %");
    REQUIRE(model.trafficText() == "In 3.4 KB/s   out 700 B/s   dropped 4");
    REQUIRE(model.rtts().values() == std::vector<float>{42.4f, 61.0f});
    REQUIRE(model.losses().values() == std::vector<float>{1.25f, 0.0f});
}

TEST_CASE("rates use a readable unit", "[client][stats]")
{
    REQUIRE(StatsModel::formatRate(0.0f) == "0 B/s");
    REQUIRE(StatsModel::formatRate(2048.0f) == "2.0 KB/s");
    REQUIRE(StatsModel::formatRate(3.0f * 1024.0f * 1024.0f) == "3.0 MB/s");
}

TEST_CASE("extra lines are replaced by name and capped", "[client][stats]")
{
    StatsModel model;

    model.setLine("Entities", "12");
    model.setLine("Entities", "15");
    for (std::size_t i = 0; i < StatsModel::MAX_EXTRA_LINES + 2; ++i)
        model.setLine("Line " + std::to_string(i), "x");

    const auto lines = model.extraLines();
    REQUIRE(lines.size() == StatsModel::MAX_EXTRA_LINES);
    REQUIRE(lines.front() == "Entities: 15");
}
