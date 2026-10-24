/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EventBus.hpp
*/

#ifndef EVENTBUS_HPP_
#define EVENTBUS_HPP_

#include <cstddef>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace engine::core {

/**
 * @brief Mediator that delivers events to the listeners of their type.
 *
 * An event is a plain value describing something that already happened.
 * Listeners of a type are called in subscription order, which keeps the
 * simulation deterministic. Publishing an event nobody listens to does
 * nothing. Not thread-safe: use it from the thread that owns the simulation.
 *
 * The bus is not reentrant yet: subscribing, unsubscribing or publishing
 * from inside a listener throws std::logic_error.
 */
class EventBus {
public:
    using ListenerId = std::size_t;

    /**
     * @brief Registers a listener for the events of type Event.
     *
     * @return An identifier, never reused by this bus.
     * @throws std::invalid_argument if the listener is empty.
     * @throws std::logic_error if called during a dispatch.
     */
    template <typename Event>
    ListenerId subscribe(std::function<void(const Event &)> listener);

    /**
     * @brief Removes a listener, the order of the others is unchanged.
     *
     * @return false if no such listener is registered.
     * @throws std::logic_error if called during a dispatch.
     */
    bool unsubscribe(ListenerId id);

    /**
     * @brief Delivers an event to every listener of its type.
     *
     * An exception thrown by a listener stops the delivery and reaches the
     * caller; the listeners after it do not see the event.
     *
     * @throws std::logic_error if called during a dispatch.
     */
    template <typename Event>
    void publish(const Event &event);

    /** @brief Number of registered listeners, all types together. */
    [[nodiscard]] std::size_t size() const noexcept;

private:
    using Callback = std::function<void(const void *)>;

    struct Listener {
        ListenerId id;
        Callback callback;
    };

    ListenerId add(std::type_index type, Callback callback);
    void dispatch(std::type_index type, const void *event);
    void ensureIdle() const;

    std::unordered_map<std::type_index, std::vector<Listener>> _listeners;
    ListenerId _nextId = 0;
    bool _dispatching = false;
};

} // namespace engine::core

#include "Engine/Core/EventBus.tpp"

#endif /* !EVENTBUS_HPP_ */
