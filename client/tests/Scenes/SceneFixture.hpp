/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneFixture.hpp
*/

#ifndef SCENEFIXTURE_HPP_
#define SCENEFIXTURE_HPP_

#include <string>

#include "Client/Scenes/MenuContext.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/SceneManager.hpp"
#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"
#include "Engine/Ecs/UIScene.hpp"

/** @brief A headless client: the input goes in, the bus and registry out. */
struct SceneFixture {
    SceneFixture()
    {
        engine::ecs::registerUIComponents(registry);
        bus.subscribe<client::scenes::NavigationRequested>(
            [this](const client::scenes::NavigationRequested &request) {
                route = request.route;
                ++navigations;
            });
        bus.subscribe<client::scenes::OverlayRequested>(
            [this](const client::scenes::OverlayRequested &request) {
                route = request.route;
                ++overlayRequests;
            });
        bus.subscribe<engine::core::QuitRequested>(
            [this](const engine::core::QuitRequested &) { ++quits; });
    }

    engine::ecs::UISceneContext context()
    {
        return client::scenes::makeMenuContext(registry, bus, interaction,
                                               "font.ttf", &theme);
    }

    void frame()
    {
        layout.update(registry);
        scenes.handleInput();
        scenes.update(1.0f / 60.0f);
    }

    void key(engine::ecs::UIKey pressed)
    {
        interaction.onKeyPressed(pressed);
        frame();
    }

    void type(const std::u32string &characters)
    {
        for (const char32_t character : characters)
            interaction.onTextEntered(character);
        frame();
    }

    void click(engine::ecs::Entity entity)
    {
        layout.update(registry);
        const auto &bounds = registry.getComponents<engine::ecs::UITransform>()
                                 .get(entity)
                                 ->actualBounds;
        const float x = bounds.x + bounds.width / 2.0f;
        const float y = bounds.y + bounds.height / 2.0f;

        interaction.onMouseDown(x, y);
        interaction.onMouseUp(x, y);
        frame();
    }

    engine::ecs::UITheme theme;
    engine::ecs::Registry registry;
    engine::core::EventBus bus;
    engine::ecs::UIInteractionSystem interaction{bus};
    engine::ecs::UILayoutSystem layout{1280.0f, 720.0f};
    engine::core::SceneManager scenes;
    client::scenes::SceneRoute route = client::scenes::SceneRoute::MAIN_MENU;
    int navigations = 0;
    int overlayRequests = 0;
    int quits = 0;
};

#endif /* !SCENEFIXTURE_HPP_ */
