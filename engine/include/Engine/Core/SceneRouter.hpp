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
#include <optional>

#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/IScene.hpp"
#include "Engine/Core/SceneManager.hpp"

namespace engine::core {

/** @brief A scene asks to leave for another screen of the game. */
template <typename Route>
struct NavigationRequested {
    Route route{};
};

/** @brief The player asked to quit the game. */
struct QuitRequested {};

/**
 * @brief Switches scenes when they ask for it, between two frames.
 *
 * Route identifies a screen, the game chooses it (usually its own enum). A
 * scene publishes NavigationRequested<Route> from its own listeners, so
 * switching right away would destroy it while it runs. The request is kept
 * until apply(), which the main loop calls after SceneManager::handleInput()
 * and update(). The last request of a frame wins, and a route without a scene
 * is ignored.
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

    /** @brief Performs the pending request, if any. */
    void apply();

    /** @brief Whether QuitRequested was published. */
    [[nodiscard]] bool quitRequested() const noexcept;

private:
    SceneManager &_scenes;
    EventBus &_bus;
    EventBus::ListenerId _navigationListener;
    EventBus::ListenerId _quitListener;
    std::map<Route, SceneFactory> _routes;
    std::optional<Route> _pending;
    bool _quit = false;
};

} // namespace engine::core

#include "Engine/Core/SceneRouter.tpp"

#endif /* !SCENEROUTER_HPP_ */
