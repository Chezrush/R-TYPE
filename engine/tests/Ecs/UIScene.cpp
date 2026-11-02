/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIScene.cpp
*/

#include "Engine/Ecs/UIScene.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <string>
#include <utility>

#include "Engine/Ecs/Components/UIComponents.hpp"

using engine::core::EventBus;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::UIInteractionSystem;
using engine::ecs::UIKey;
using engine::ecs::UIScene;
using engine::ecs::UISceneContext;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIText;

namespace {

/** @brief A screen that is not from any game: one button and a label. */
class ShopScene : public UIScene {
public:
    using UIScene::UIScene;

    [[nodiscard]] bool active() const noexcept
    {
        return isActive();
    }

    int activations = 0;
    int clicks = 0;
    std::optional<Entity> buy;

protected:
    void activate() override
    {
        ++activations;
        addLabel("Shop", {0.0f, -100.0f}, {200.0f, 40.0f}, 32);
        buy = addButton("Buy", "shop.buy", {0.0f, 0.0f}, {200.0f, 40.0f});
        widget<UIButton>(*buy).isFocused = true;
        listen<UIButtonClickedEvent>(
            [this](const UIButtonClickedEvent &) { ++clicks; });
    }
};

struct SceneWorld {
    SceneWorld()
    {
        engine::ecs::registerUIComponents(registry);
    }

    UISceneContext context()
    {
        return {registry, bus, interaction, "font.ttf"};
    }

    Registry registry;
    EventBus bus;
    UIInteractionSystem interaction{bus};
};

} // namespace

TEST_CASE("a UI scene has no widgets before it enters", "[ecs][ui][scene]")
{
    SceneWorld world;
    const ShopScene scene(world.context());

    REQUIRE(world.registry.aliveCount() == 0);
    REQUIRE_FALSE(scene.isOverlay());
}

TEST_CASE("a UI scene gets input through handleInput", "[ecs][ui][scene]")
{
    SceneWorld world;
    ShopScene scene(world.context());

    scene.onEnter();
    world.interaction.onKeyPressed(UIKey::ENTER);
    REQUIRE(scene.clicks == 0);

    scene.handleInput();
    REQUIRE(scene.clicks == 1);
}

TEST_CASE("exit removes the widgets and the listeners", "[ecs][ui][scene]")
{
    SceneWorld world;
    const Entity kept = world.registry.spawnEntity();
    ShopScene scene(world.context());

    scene.onEnter();
    REQUIRE(world.registry.aliveCount() == 3);
    scene.onExit();

    REQUIRE(world.registry.aliveCount() == 1);
    REQUIRE(world.registry.isAlive(kept));
    world.bus.publish(UIButtonClickedEvent{0, "shop.buy"});
    REQUIRE(scene.clicks == 0);
}

TEST_CASE("pause removes everything and resume builds it again",
          "[ecs][ui][scene]")
{
    SceneWorld world;
    ShopScene scene(world.context());

    scene.onEnter();
    scene.onPause();
    REQUIRE(world.registry.aliveCount() == 0);

    scene.onResume();
    REQUIRE(scene.activations == 2);
    REQUIRE(world.registry.aliveCount() == 2);
    world.bus.publish(UIButtonClickedEvent{0, "shop.buy"});
    REQUIRE(scene.clicks == 1);
}

TEST_CASE("a destroyed UI scene cleans up after itself", "[ecs][ui][scene]")
{
    SceneWorld world;
    {
        ShopScene scene(world.context());
        scene.onEnter();
    }
    REQUIRE(world.registry.aliveCount() == 0);
}

TEST_CASE("the game chooses the look of the buttons", "[ecs][ui][scene]")
{
    SceneWorld world;
    UISceneContext context = world.context();
    context.buttonStyle.normalColor = UIColor{1, 2, 3, 255};
    context.buttonTextSize = 40;
    ShopScene scene(std::move(context));

    scene.onEnter();

    const auto &button =
        *world.registry.getComponents<UIButton>().get(*scene.buy);
    const auto &label = *world.registry.getComponents<UIText>().get(*scene.buy);
    REQUIRE(button.normalColor == UIColor{1, 2, 3, 255});
    REQUIRE(button.action == "shop.buy");
    REQUIRE(label.text == "Buy");
    REQUIRE(label.fontId == "font.ttf");
    REQUIRE(label.characterSize == 40);
}

TEST_CASE("a UI scene knows whether its widgets exist", "[ecs][ui][scene]")
{
    SceneWorld world;
    ShopScene scene(world.context());

    REQUIRE_FALSE(scene.active());
    scene.onEnter();
    REQUIRE(scene.active());
    scene.onPause();
    REQUIRE_FALSE(scene.active());
    scene.onResume();
    REQUIRE(scene.active());
    scene.onExit();
    REQUIRE_FALSE(scene.active());
}
