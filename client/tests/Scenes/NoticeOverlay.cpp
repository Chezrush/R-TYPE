/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NoticeOverlay.cpp
*/

#include "Client/Scenes/NoticeOverlay.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "SceneFixture.hpp"

using client::scenes::NoticeOverlay;
using engine::ecs::UIKey;

namespace {

NoticeOverlay &enterNotice(SceneFixture &fixture, int &closes)
{
    auto scene =
        std::make_unique<NoticeOverlay>(fixture.context(), "Connection lost");
    NoticeOverlay &notice = *scene;

    fixture.bus.subscribe<engine::core::OverlayCloseRequested>(
        [&closes](const engine::core::OverlayCloseRequested &) { ++closes; });
    fixture.scenes.pushScene(std::move(scene));
    fixture.frame();
    return notice;
}

} // namespace

TEST_CASE("the notice keeps its message and closes with OK",
          "[client][scenes][notice]")
{
    SceneFixture fixture;
    int closes = 0;
    NoticeOverlay &notice = enterNotice(fixture, closes);

    REQUIRE(notice.message() == "Connection lost");
    fixture.click(notice.okButton());
    REQUIRE(closes == 1);
}

TEST_CASE("Escape closes the notice", "[client][scenes][notice]")
{
    SceneFixture fixture;
    int closes = 0;

    enterNotice(fixture, closes);
    fixture.key(UIKey::ESCAPE);
    REQUIRE(closes == 1);
}
