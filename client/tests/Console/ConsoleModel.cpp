/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleModel.cpp
*/

#include "Client/Console/ConsoleModel.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

using client::console::Completion;
using client::console::ConsoleModel;
using client::events::ConsoleLevel;

namespace {

ConsoleModel withHistory()
{
    ConsoleModel model;

    static_cast<void>(model.submit("first"));
    static_cast<void>(model.submit("second"));
    return model;
}

} // namespace

TEST_CASE("submitting trims the line and skips empty ones", "[client][console]")
{
    ConsoleModel model;

    REQUIRE(model.submit("  god mode  ") == "god mode");
    REQUIRE_FALSE(model.submit("   ").has_value());
    REQUIRE(model.history() == std::vector<std::string>{"god mode"});
}

TEST_CASE("the same command twice is kept once", "[client][console]")
{
    ConsoleModel model;

    static_cast<void>(model.submit("fps"));
    static_cast<void>(model.submit("fps"));
    static_cast<void>(model.submit("quit"));
    static_cast<void>(model.submit("fps"));

    REQUIRE(model.history() == std::vector<std::string>{"fps", "quit", "fps"});
}

TEST_CASE("Up goes back in time and stops at the oldest", "[client][console]")
{
    ConsoleModel model = withHistory();

    REQUIRE(model.previous("typing") == "second");
    REQUIRE(model.previous("second") == "first");
    REQUIRE_FALSE(model.previous("first").has_value());
}

TEST_CASE("Down comes back to the line being typed", "[client][console]")
{
    ConsoleModel model = withHistory();

    static_cast<void>(model.previous("typing"));
    static_cast<void>(model.previous("second"));

    REQUIRE(model.next() == "second");
    REQUIRE(model.next() == "typing");
    REQUIRE_FALSE(model.next().has_value());
}

TEST_CASE("an empty history gives nothing", "[client][console]")
{
    ConsoleModel model;

    REQUIRE_FALSE(model.previous("x").has_value());
    REQUIRE_FALSE(model.next().has_value());
}

TEST_CASE("submitting restarts the browsing", "[client][console]")
{
    ConsoleModel model = withHistory();

    static_cast<void>(model.previous(""));
    static_cast<void>(model.submit("third"));

    REQUIRE(model.previous("") == "third");
}

TEST_CASE("the history keeps the latest entries", "[client][console]")
{
    ConsoleModel model;

    for (std::size_t i = 0; i < ConsoleModel::MAX_HISTORY + 5; ++i)
        static_cast<void>(model.submit("cmd" + std::to_string(i)));

    REQUIRE(model.history().size() == ConsoleModel::MAX_HISTORY);
    REQUIRE(model.history().front() == "cmd5");
}

TEST_CASE("a unique match completes the command", "[client][console]")
{
    ConsoleModel model;
    model.setCommands({"spawn", "quit", "score"});

    REQUIRE(model.complete("sp") == Completion{"spawn ", {}});
    REQUIRE(model.complete("x") == Completion{"x", {}});
}

TEST_CASE("several matches give their common start and the list",
          "[client][console]")
{
    ConsoleModel model;
    model.setCommands({"show_fps", "show_hitboxes", "quit", "show_fps"});

    const Completion completion = model.complete("s");

    REQUIRE(completion.text == "show_");
    REQUIRE(completion.candidates ==
            std::vector<std::string>{"show_fps", "show_hitboxes"});
}

TEST_CASE("only the command name is completed", "[client][console]")
{
    ConsoleModel model;
    model.setCommands({"spawn"});

    REQUIRE(model.complete("spawn sp") == Completion{"spawn sp", {}});
}

TEST_CASE("output lines are prefixed by their level", "[client][console]")
{
    ConsoleModel model;

    model.addOutput("ready", ConsoleLevel::INFO);
    model.addOutput("slow", ConsoleLevel::WARNING);
    model.addOutput("unknown command", ConsoleLevel::ERROR_LINE);

    REQUIRE(model.lines()[0] == "ready");
    REQUIRE(model.lines()[1] == "[warning] slow");
    REQUIRE(model.lines()[2] == "[error] unknown command");
}

TEST_CASE("the log keeps the latest lines", "[client][console]")
{
    ConsoleModel model;

    for (std::size_t i = 0; i < ConsoleModel::MAX_LINES + 1; ++i)
        model.addOutput(std::to_string(i), ConsoleLevel::INFO);

    REQUIRE(model.lines().size() == ConsoleModel::MAX_LINES);
    REQUIRE(model.lines().front() == "1");
}
