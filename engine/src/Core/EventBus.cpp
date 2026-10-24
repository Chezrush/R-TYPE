/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EventBus.cpp
*/

#include "Engine/Core/EventBus.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace engine::core {

EventBus::ListenerId EventBus::add(std::type_index type, Callback callback)
{
    ensureIdle();
    const ListenerId id = _nextId++;

    _listeners[type].push_back({id, std::move(callback)});
    return id;
}

bool EventBus::unsubscribe(ListenerId id)
{
    ensureIdle();
    for (auto &entry : _listeners) {
        auto &listeners = entry.second;
        const auto found =
            std::find_if(listeners.begin(), listeners.end(),
                         [id](const Listener &item) { return item.id == id; });
        if (found == listeners.end())
            continue;
        listeners.erase(found);
        return true;
    }
    return false;
}

void EventBus::dispatch(std::type_index type, const void *event)
{
    ensureIdle();
    const auto found = _listeners.find(type);

    if (found == _listeners.end())
        return;
    _dispatching = true;
    try {
        for (const Listener &listener : found->second)
            listener.callback(event);
    } catch (...) {
        _dispatching = false;
        throw;
    }
    _dispatching = false;
}

std::size_t EventBus::size() const noexcept
{
    std::size_t total = 0;

    for (const auto &entry : _listeners)
        total += entry.second.size();
    return total;
}

void EventBus::ensureIdle() const
{
    if (_dispatching)
        throw std::logic_error("event bus used during a dispatch");
}

} // namespace engine::core
