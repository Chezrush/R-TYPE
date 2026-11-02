/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneRouter.hpp
*/

#ifndef SCENEROUTER_HPP_
#define SCENEROUTER_HPP_

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/IScene.hpp"
#include "Engine/Core/SceneManager.hpp"

namespace engine::core {

/** @brief A scene asks to leave for another screen of the game. */
template <typename Route>
struct NavigationRequested {
    Route route{};
};

/** @brief A scene asks to show an overlay above the current scene. */
template <typename Route>
struct OverlayRequested {
    Route route{};
};

/** @brief A scene asks to close the overlay on top of the stack. */
struct OverlayCloseRequested {};

/** @brief The player asked to quit the game. */
struct QuitRequested {};

/**
 * @brief Switches scenes when they ask for it, between two frames.
 *
 * Route identifies a screen, the game chooses it (usually its own enum). A
 * scene publishes NavigationRequested<Route> from its own listeners, so
 * switching right away would destroy it while it runs. The request is kept
 * until apply(), which the main loop calls after SceneManager::handleInput()
 * and update(). OverlayRequested and OverlayCloseRequested are kept the same
 * way and applied in order. A navigation drops the requests made before it,
 * since it replaces the whole stack: the last navigation of a frame wins. A
 * route without a scene is ignored, and closing only ever removes an overlay.
 */
template <typename Route>
class SceneRouter {
public:
    using SceneFactory = std::function<std::unique_ptr<IScene>()>;

    SceneRouter(SceneManager &scenes, EventBus &bus);
    ~SceneRouter();

    SceneRouter(const SceneRouter &) = delete;
    SceneRouter &operator=(const SceneRouter &) = delete;
    SceneRouter(SceneRouter &&) = delete;
    SceneRouter &operator=(SceneRouter &&) = delete;

    /** @brief Sets the scene a route leads to. */
    void setRoute(Route route, SceneFactory factory);

    /** @brief Performs the pending requests, if any. */
    void apply();

    /** @brief Whether QuitRequested was published. */
    [[nodiscard]] bool quitRequested() const noexcept;

private:
    [[nodiscard]] std::unique_ptr<IScene> create(const Route &route) const;
    void requestNavigation(const Route &route);
    void requestOverlay(const Route &route);
    void closeOverlay();

    SceneManager &_scenes;
    EventBus &_bus;
    std::vector<EventBus::ListenerId> _listeners;
    std::map<Route, SceneFactory> _routes;
    std::vector<std::function<void()>> _pending;
    bool _quit = false;
};

} // namespace engine::core

#include "Engine/Core/SceneRouter.tpp"

#endif /* !SCENEROUTER_HPP_ */
