/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UILayoutSystem.cpp
*/

#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "Engine/Ecs/EntityAllocator.hpp"
#include "Engine/Ecs/SparseArray.hpp"

using engine::ecs::EntityAllocator;
using engine::ecs::SparseArray;
using engine::ecs::components::Rect;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UITransform;
using engine::ecs::components::Vec2;
using engine::ecs::systems::UILayoutSystem;

TEST_CASE("all 9 anchors resolve accurately at 1280x720", "[ecs][ui][layout]")
{
    constexpr float width = 1280.0f;
    constexpr float height = 720.0f;

    SECTION("TOP_LEFT anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::TOP_LEFT;
        t.offset = {10.0f, 15.0f};
        t.size = {100.0f, 50.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{10.0f, 15.0f, 100.0f, 50.0f});
    }

    SECTION("TOP_CENTER anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::TOP_CENTER;
        t.offset = {0.0f, 10.0f};
        t.size = {200.0f, 50.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{540.0f, 10.0f, 200.0f, 50.0f});
    }

    SECTION("TOP_RIGHT anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::TOP_RIGHT;
        t.offset = {-20.0f, 10.0f};
        t.size = {150.0f, 40.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{1110.0f, 10.0f, 150.0f, 40.0f});
    }

    SECTION("CENTER_LEFT anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::CENTER_LEFT;
        t.offset = {10.0f, 0.0f};
        t.size = {50.0f, 200.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{10.0f, 260.0f, 50.0f, 200.0f});
    }

    SECTION("CENTER anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::CENTER;
        t.offset = {5.0f, -5.0f};
        t.size = {300.0f, 100.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{495.0f, 305.0f, 300.0f, 100.0f});
    }

    SECTION("CENTER_RIGHT anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::CENTER_RIGHT;
        t.offset = {-10.0f, 0.0f};
        t.size = {50.0f, 200.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{1220.0f, 260.0f, 50.0f, 200.0f});
    }

    SECTION("BOTTOM_LEFT anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::BOTTOM_LEFT;
        t.offset = {10.0f, -10.0f};
        t.size = {100.0f, 50.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{10.0f, 660.0f, 100.0f, 50.0f});
    }

    SECTION("BOTTOM_CENTER anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::BOTTOM_CENTER;
        t.offset = {0.0f, -20.0f};
        t.size = {800.0f, 60.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{240.0f, 640.0f, 800.0f, 60.0f});
    }

    SECTION("BOTTOM_RIGHT anchor")
    {
        UITransform t;
        t.anchor = UIAnchor::BOTTOM_RIGHT;
        t.offset = {-15.0f, -15.0f};
        t.size = {120.0f, 60.0f};

        const Rect bounds = UILayoutSystem::computeBounds(t, width, height);
        REQUIRE(bounds == Rect{1145.0f, 645.0f, 120.0f, 60.0f});
    }
}

TEST_CASE("window resize recalculates bounds without drift",
          "[ecs][ui][layout]")
{
    UILayoutSystem system(1280.0f, 720.0f);

    UITransform hud;
    hud.anchor = UIAnchor::BOTTOM_CENTER;
    hud.offset = {0.0f, -20.0f};
    hud.size = {800.0f, 60.0f};

    const Rect bounds720p =
        system.computeBounds(hud, system.windowWidth(), system.windowHeight());
    REQUIRE(bounds720p == Rect{240.0f, 640.0f, 800.0f, 60.0f});

    system.setResolution(1920.0f, 1080.0f);
    const Rect bounds1080p =
        system.computeBounds(hud, system.windowWidth(), system.windowHeight());
    REQUIRE(bounds1080p == Rect{560.0f, 1000.0f, 800.0f, 60.0f});
}

TEST_CASE("normalized size scales proportionally to resolution",
          "[ecs][ui][layout]")
{
    UITransform bar;
    bar.anchor = UIAnchor::TOP_CENTER;
    bar.size = {0.5f, 0.1f};
    bar.normalizedSize = true;

    const Rect bounds720p =
        UILayoutSystem::computeBounds(bar, 1280.0f, 720.0f);
    REQUIRE(bounds720p.width == 640.0f);
    REQUIRE(bounds720p.height == 72.0f);
    REQUIRE(bounds720p.x == 320.0f);
    REQUIRE(bounds720p.y == 0.0f);

    const Rect bounds1080p =
        UILayoutSystem::computeBounds(bar, 1920.0f, 1080.0f);
    REQUIRE(bounds1080p.width == 960.0f);
    REQUIRE(bounds1080p.height == 108.0f);
    REQUIRE(bounds1080p.x == 480.0f);
    REQUIRE(bounds1080p.y == 0.0f);
}

TEST_CASE("z-index comparison sorts UI transforms", "[ecs][ui][layout]")
{
    UITransform bg;
    bg.zIndex = 0;

    UITransform panel;
    panel.zIndex = 10;

    UITransform tooltip;
    tooltip.zIndex = 100;

    std::vector<UITransform> list = {tooltip, bg, panel};
    std::sort(list.begin(), list.end());

    REQUIRE(list[0].zIndex == 0);
    REQUIRE(list[1].zIndex == 10);
    REQUIRE(list[2].zIndex == 100);
}

TEST_CASE("UILayoutSystem updates sparse array components in-place",
          "[ecs][ui][layout]")
{
    EntityAllocator allocator;
    SparseArray<UITransform> transforms;
    UILayoutSystem system(1280.0f, 720.0f);

    const auto e1 = allocator.create();
    const auto e2 = allocator.create();

    UITransform t1;
    t1.anchor = UIAnchor::TOP_LEFT;
    t1.offset = {50.0f, 50.0f};
    t1.size = {100.0f, 100.0f};

    UITransform t2;
    t2.anchor = UIAnchor::BOTTOM_RIGHT;
    t2.offset = {-50.0f, -50.0f};
    t2.size = {100.0f, 100.0f};

    transforms.insert(e1, t1);
    transforms.insert(e2, t2);

    system.update(transforms);

    REQUIRE(transforms.get(e1)->actualBounds ==
            Rect{50.0f, 50.0f, 100.0f, 100.0f});
    REQUIRE(transforms.get(e2)->actualBounds ==
            Rect{1130.0f, 570.0f, 100.0f, 100.0f});
}

