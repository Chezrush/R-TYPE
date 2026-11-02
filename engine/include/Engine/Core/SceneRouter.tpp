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
      _bus(bus),
      _navigationListener(_bus.subscribe<NavigationRequested<Route>>(
          [this](const NavigationRequested<Route> &request) {
              _pending = request.route;
          })),
      _quitListener(_bus.subscribe<QuitRequested>(
          [this](const QuitRequested &) { _quit = true; }))
{}

template <typename Route>
SceneRouter<Route>::~SceneRouter()
{
    _bus.unsubscribe(_navigationListener);
    _bus.unsubscribe(_quitListener);
}

template <typename Route>
void SceneRouter<Route>::setRoute(Route route, SceneFactory factory)
{
    _routes.insert_or_assign(std::move(route), std::move(factory));
}

template <typename Route>
void SceneRouter<Route>::apply()
{
    if (!_pending.has_value())
        return;
    const auto found = _routes.find(*_pending);

    _pending.reset();
    if (found != _routes.end())
        _scenes.switchScene(found->second());
}

template <typename Route>
bool SceneRouter<Route>::quitRequested() const noexcept
{
    return _quit;
}

} // namespace engine::core

#endif /* !SCENEROUTER_TPP_ */
