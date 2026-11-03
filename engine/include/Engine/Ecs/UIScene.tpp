/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIScene.tpp
*/

#ifndef UISCENE_TPP_
#define UISCENE_TPP_

#include <utility>

#include "Engine/Ecs/UIScene.hpp"

namespace engine::ecs {

template <typename Event>
void UIScene::listen(std::function<void(const Event &)> listener)
{
    _listeners.push_back(_context.bus.subscribe<Event>(std::move(listener)));
}

template <typename Component>
Component &UIScene::widget(engine::ecs::Entity entity)
{
    return *_context.registry.getComponents<Component>().get(entity);
}

} // namespace engine::ecs

#endif /* !UISCENE_TPP_ */
