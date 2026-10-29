/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EventBus.cpp
*/

#include "Engine/Core/EventBus.hpp"

#include <algorithm>
#include <utility>

namespace engine::core {

EventBus::ListenerId EventBus::add(std::type_index type, Callback callback)
{
    const ListenerId id = _nextId++;

    _listeners[type].push_back(
        std::make_shared<Listener>(Listener{id, std::move(callback), true}));
    return id;
}

bool EventBus::unsubscribe(ListenerId id)
{
    for (auto &entry : _listeners) {
        const auto found = std::find_if(
            entry.second.begin(), entry.second.end(),
            [id](const auto &item) { return item->active && item->id == id; });
        if (found == entry.second.end())
            continue;
        (*found)->active = false;
        if (!_dispatching)
            compact();
        return true;
    }
    return false;
}

void EventBus::enqueue(std::type_index type, std::shared_ptr<const void> event)
{
    _pending.push({type, std::move(event)});
}

void EventBus::run(std::type_index type, const void *event)
{
    _dispatching = true;
    try {
        deliver(type, event);
        drain();
    } catch (...) {
        finish();
        throw;
    }
    finish();
}

void EventBus::deliver(std::type_index type, const void *event)
{
    const auto found = _listeners.find(type);

    if (found == _listeners.end())
        return;
    auto &listeners = found->second;
    const std::size_t count = listeners.size();
    for (std::size_t i = 0; i < count; ++i) {
        const std::shared_ptr<Listener> listener = listeners[i];
        if (listener->active)
            listener->callback(event);
    }
}

void EventBus::drain()
{
    while (!_pending.empty()) {
        const Pending next = std::move(_pending.front());
        _pending.pop();
        deliver(next.type, next.event.get());
    }
}

void EventBus::finish()
{
    _pending = {};
    _dispatching = false;
    compact();
}

void EventBus::compact()
{
    for (auto &entry : _listeners)
        std::erase_if(entry.second,
                      [](const auto &item) { return !item->active; });
}

std::size_t EventBus::size() const noexcept
{
    std::size_t total = 0;

    for (const auto &entry : _listeners)
        total += static_cast<std::size_t>(
            std::count_if(entry.second.begin(), entry.second.end(),
                          [](const auto &item) { return item->active; }));
    return total;
}

} // namespace engine::core
