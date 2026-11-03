/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneRouter.tpp
*/

#ifndef SCENEROUTER_TPP_
#define SCENEROUTER_TPP_

#include <utility>

#include "Engine/Core/SceneRouter.hpp"

namespace engine::core {

template <typename Route>
SceneRouter<Route>::SceneRouter(SceneManager &scenes, EventBus &bus)
    : _scenes(scenes),
      _bus(bus)
{
    _listeners.push_back(_bus.subscribe<NavigationRequested<Route>>(
        [this](const NavigationRequested<Route> &request) {
            requestNavigation(request.route);
        }));
    _listeners.push_back(_bus.subscribe<OverlayRequested<Route>>(
        [this](const OverlayRequested<Route> &request) {
            requestOverlay(request.route);
        }));
    _listeners.push_back(_bus.subscribe<OverlayCloseRequested>(
        [this](const OverlayCloseRequested &) {
            _pending.emplace_back([this] { closeOverlay(); });
        }));
    _listeners.push_back(_bus.subscribe<OverlaysClearRequested>(
        [this](const OverlaysClearRequested &) {
            _pending.emplace_back([this] { closeOverlays(); });
        }));
    _listeners.push_back(_bus.subscribe<QuitRequested>(
        [this](const QuitRequested &) { _quit = true; }));
}

template <typename Route>
SceneRouter<Route>::~SceneRouter()
{
    for (const auto listener : _listeners)
        _bus.unsubscribe(listener);
}

template <typename Route>
void SceneRouter<Route>::setRoute(Route route, SceneFactory factory)
{
    _routes.insert_or_assign(std::move(route), std::move(factory));
}

template <typename Route>
void SceneRouter<Route>::apply()
{
    const std::vector<std::function<void()>> pending = std::move(_pending);

    _pending.clear();
    for (const auto &request : pending)
        request();
}

template <typename Route>
void SceneRouter<Route>::requestNavigation(const Route &route)
{
    _pending.clear();
    _pending.emplace_back([this, route] {
        if (auto scene = create(route))
            _scenes.switchScene(std::move(scene));
    });
}

template <typename Route>
void SceneRouter<Route>::requestOverlay(const Route &route)
{
    _pending.emplace_back([this, route] { _scenes.pushScene(create(route)); });
}

template <typename Route>
std::unique_ptr<IScene> SceneRouter<Route>::create(const Route &route) const
{
    const auto found = _routes.find(route);

    return found != _routes.end() ? found->second() : nullptr;
}

template <typename Route>
void SceneRouter<Route>::closeOverlay()
{
    if (!_scenes.empty() && _scenes.currentScene().isOverlay())
        _scenes.popScene();
}

template <typename Route>
void SceneRouter<Route>::closeOverlays()
{
    while (!_scenes.empty() && _scenes.currentScene().isOverlay())
        _scenes.popScene();
}

template <typename Route>
bool SceneRouter<Route>::quitRequested() const noexcept
{
    return _quit;
}

} // namespace engine::core

#endif /* !SCENEROUTER_TPP_ */
