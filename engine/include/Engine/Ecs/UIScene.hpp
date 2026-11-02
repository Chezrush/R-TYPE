/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIScene.hpp
*/

#ifndef UISCENE_HPP_
#define UISCENE_HPP_

#include <functional>
#include <string>
#include <vector>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/IScene.hpp"
#include "Engine/Core/Vec2.hpp"
#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UIColor.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"

namespace engine::ecs {

/**
 * @brief What a UI scene needs from the game.
 *
 * The registry, the bus and the interaction system must outlive the scene. The
 * window code feeds the interaction system; the registry is drawn by the game's
 * UIRenderSystem. buttonStyle is copied into every button of addButton(), so
 * the colors belong to the game.
 */
struct UISceneContext {
    engine::ecs::Registry &registry;
    engine::core::EventBus &bus;
    engine::ecs::UIInteractionSystem &interaction;
    std::string fontId;
    engine::core::Vec2 viewport{1280.0f, 720.0f};
    engine::ecs::components::UIButton buttonStyle{};
    unsigned int buttonTextSize = 24;
};

/**
 * @brief Base of the screens built only from UI components (menus, forms...).
 *
 * The widgets and the listeners only exist while the scene is on top: they are
 * created by activate() on enter / resume, and removed on exit / pause. So a
 * scene below another one neither draws nor reacts. render() does nothing,
 * since the widgets are entities drawn with the rest of the registry.
 */
class UIScene : public engine::core::IScene {
public:
    explicit UIScene(UISceneContext context);
    ~UIScene() override;

    UIScene(const UIScene &) = delete;
    UIScene &operator=(const UIScene &) = delete;
    UIScene(UIScene &&) = delete;
    UIScene &operator=(UIScene &&) = delete;

    void onEnter() override;
    void onExit() override;
    void onPause() override;
    void onResume() override;
    void update(float dt) override;
    void render() override;
    void handleInput() override;
    [[nodiscard]] bool isOverlay() const noexcept override;

    /** @brief Tells the scene the new window size. */
    void setViewport(engine::core::Vec2 viewport) noexcept;

protected:
    /** @brief Creates the widgets and the listeners of the scene. */
    virtual void activate() = 0;

    engine::ecs::Entity spawn(engine::ecs::components::UIAnchor anchor,
                              engine::core::Vec2 offset,
                              engine::core::Vec2 size, int zIndex = 0);
    engine::ecs::Entity addLabel(const std::string &text,
                                 engine::core::Vec2 offset,
                                 engine::core::Vec2 size,
                                 unsigned int characterSize);
    engine::ecs::Entity addButton(const std::string &text,
                                  const std::string &action,
                                  engine::core::Vec2 offset,
                                  engine::core::Vec2 size);

    /** @brief Subscribes a listener, removed when the scene is deactivated. */
    template <typename Event>
    void listen(std::function<void(const Event &)> listener);

    /** @brief A component of one of the widgets of this scene. */
    template <typename Component>
    [[nodiscard]] Component &widget(engine::ecs::Entity entity);

    [[nodiscard]] engine::core::EventBus &bus() noexcept;
    [[nodiscard]] engine::ecs::Registry &registry() noexcept;
    [[nodiscard]] const std::string &fontId() const noexcept;
    [[nodiscard]] engine::core::Vec2 viewport() const noexcept;

private:
    void deactivate();

    UISceneContext _context;
    std::vector<engine::ecs::Entity> _entities;
    std::vector<engine::core::EventBus::ListenerId> _listeners;
};

} // namespace engine::ecs

#include "Engine/Ecs/UIScene.tpp"

#endif /* !UISCENE_HPP_ */
