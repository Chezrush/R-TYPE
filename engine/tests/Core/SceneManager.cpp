/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneManager.cpp
*/

#include "Engine/Core/SceneManager.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using engine::core::IScene;
using engine::core::SceneManager;

namespace {

class MockScene : public IScene {
public:
    explicit MockScene(
        std::string name,
        std::vector<std::string> *events = nullptr,
        bool overlay = false
    )
        : _name(std::move(name)),
          _events(events),
          _overlay(overlay)
    {}

    void onEnter() override
    {
        record("enter:" + _name);
    }

    void onExit() override
    {
        record("exit:" + _name);
    }

    void onPause() override
    {
        record("pause:" + _name);
    }

    void onResume() override
    {
        record("resume:" + _name);
    }

    void update(float dt) override
    {
        _lastDt = dt;
        record("update:" + _name);
    }

    void render() override
    {
        record("render:" + _name);
    }

    void handleInput() override
    {
        record("input:" + _name);
    }

    [[nodiscard]] bool isOverlay() const noexcept override
    {
        return _overlay;
    }

    [[nodiscard]] float lastDt() const noexcept
    {
        return _lastDt;
    }

private:
    void record(const std::string &event)
    {
        if (_events != nullptr) {
            _events->push_back(event);
        }
    }

    std::string _name;
    std::vector<std::string> *_events;
    bool _overlay;
    float _lastDt = 0.0f;
};

} // namespace

TEST_CASE("scene manager starts empty", "[core][scenemanager]")
{
    SceneManager manager;

    REQUIRE(manager.empty());
    REQUIRE(manager.size() == 0);
    REQUIRE_THROWS_AS(manager.currentScene(), std::runtime_error);
}

TEST_CASE("pushing a scene invokes onEnter and updates size",
          "[core][scenemanager]")
{
    std::vector<std::string> events;
    SceneManager manager;

    manager.pushScene(std::make_unique<MockScene>("menu", &events));
    REQUIRE_FALSE(manager.empty());
    REQUIRE(manager.size() == 1);
    REQUIRE(events == std::vector<std::string>{"enter:menu"});
}

TEST_CASE("pushing a second scene pauses the first and enters second",
          "[core][scenemanager]")
{
    std::vector<std::string> events;
    SceneManager manager;

    manager.pushScene(std::make_unique<MockScene>("game", &events));
    manager.pushScene(std::make_unique<MockScene>("pause", &events, true));

    REQUIRE(manager.size() == 2);
    const std::vector<std::string> expected = {
        "enter:game",
        "pause:game",
        "enter:pause"
    };
    REQUIRE(events == expected);
}

TEST_CASE("popping a scene exits top and resumes previous",
          "[core][scenemanager]")
{
    std::vector<std::string> events;
    SceneManager manager;

    manager.pushScene(std::make_unique<MockScene>("game", &events));
    manager.pushScene(std::make_unique<MockScene>("pause", &events, true));
    events.clear();

    manager.popScene();
    REQUIRE(manager.size() == 1);
    const std::vector<std::string> expected = {
        "exit:pause",
        "resume:game"
    };
    REQUIRE(events == expected);
}

TEST_CASE("switchScene clears previous stack and enters new scene",
          "[core][scenemanager]")
{
    std::vector<std::string> events;
    SceneManager manager;

    manager.pushScene(std::make_unique<MockScene>("game", &events));
    manager.pushScene(std::make_unique<MockScene>("pause", &events, true));
    events.clear();

    manager.switchScene(std::make_unique<MockScene>("menu", &events));
    REQUIRE(manager.size() == 1);
    const std::vector<std::string> expected = {
        "exit:pause",
        "exit:game",
        "enter:menu"
    };
    REQUIRE(events == expected);
}

TEST_CASE("update and handleInput only reach the active top scene",
          "[core][scenemanager]")
{
    std::vector<std::string> events;
    SceneManager manager;

    manager.pushScene(std::make_unique<MockScene>("game", &events));
    manager.pushScene(std::make_unique<MockScene>("pause", &events, true));
    events.clear();

    manager.update(0.016f);
    manager.handleInput();

    const std::vector<std::string> expected = {
        "update:pause",
        "input:pause"
    };
    REQUIRE(events == expected);
}

TEST_CASE("render draws overlay on top of underlying scene",
          "[core][scenemanager]")
{
    std::vector<std::string> events;
    SceneManager manager;

    manager.pushScene(std::make_unique<MockScene>("game", &events));
    manager.pushScene(std::make_unique<MockScene>("pause", &events, true));
    events.clear();

    manager.render();
    const std::vector<std::string> expected = {
        "render:game",
        "render:pause"
    };
    REQUIRE(events == expected);
}

TEST_CASE("render draws only top scene if top is not an overlay",
          "[core][scenemanager]")
{
    std::vector<std::string> events;
    SceneManager manager;

    manager.pushScene(std::make_unique<MockScene>("menu", &events));
    manager.pushScene(std::make_unique<MockScene>("settings", &events, false));
    events.clear();

    manager.render();
    const std::vector<std::string> expected = {
        "render:settings"
    };
    REQUIRE(events == expected);
}

TEST_CASE("clear exits all scenes", "[core][scenemanager]")
{
    std::vector<std::string> events;
    {
        SceneManager manager;
        manager.pushScene(std::make_unique<MockScene>("game", &events));
        manager.pushScene(std::make_unique<MockScene>("pause", &events, true));
        events.clear();
        manager.clear();
        REQUIRE(manager.empty());
    }
    const std::vector<std::string> expected = {
        "exit:pause",
        "exit:game"
    };
    REQUIRE(events == expected);
}

