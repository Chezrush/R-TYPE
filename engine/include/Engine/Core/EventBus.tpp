/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EventBus.tpp
*/

#ifndef EVENTBUS_TPP_
#define EVENTBUS_TPP_

#include <stdexcept>
#include <utility>

#include "Engine/Core/EventBus.hpp"

namespace engine::core {

template <typename Event>
EventBus::ListenerId
EventBus::subscribe(std::function<void(const Event &)> listener)
{
    if (!listener)
        throw std::invalid_argument("invalid listener");
    return add(typeid(Event),
               [listener = std::move(listener)](const void *event) {
                   listener(*static_cast<const Event *>(event));
               });
}

template <typename Event>
void EventBus::publish(const Event &event)
{
    dispatch(typeid(Event), &event);
}

} // namespace engine::core

#endif /* !EVENTBUS_TPP_ */
