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
#include <memory>
#include <queue>
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
 * Rules when the bus is used from inside a listener:
 * - An event published during a dispatch is copied and queued. It is
 *   delivered after the current event is done, in publication order, and
 *   publish() returns without waiting for it.
 * - A listener unsubscribed during a dispatch is not called again, not even
 *   for the event being dispatched.
 * - A listener subscribed during a dispatch does not receive the event being
 *   dispatched, only the ones delivered after it.
 * - If a listener throws, the exception reaches the caller of the outermost
 *   publish(), the events still queued are discarded and the bus stays
 *   usable.
 */
class EventBus {
public:
    using ListenerId = std::size_t;

    /**
     * @brief Registers a listener for the events of type Event.
     *
     * @return An identifier, never reused by this bus.
     * @throws std::invalid_argument if the listener is empty.
     */
    template <typename Event>
    ListenerId subscribe(std::function<void(const Event &)> listener);

    /**
     * @brief Removes a listener, the order of the others is unchanged.
     *
     * @return false if no such listener is registered.
     */
    bool unsubscribe(ListenerId id);

    /**
     * @brief Delivers an event to every listener of its type.
     *
     * Called from a listener, the event is queued instead (see the class
     * description), which requires Event to be copy-constructible.
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
        bool active;
    };

    struct Pending {
        std::type_index type;
        std::shared_ptr<const void> event;
    };

    ListenerId add(std::type_index type, Callback callback);
    void enqueue(std::type_index type, std::shared_ptr<const void> event);
    void run(std::type_index type, const void *event);
    void deliver(std::type_index type, const void *event);
    void drain();
    void finish();
    void compact();

    std::unordered_map<std::type_index, std::vector<std::shared_ptr<Listener>>>
        _listeners;
    std::queue<Pending> _pending;
    ListenerId _nextId = 0;
    bool _dispatching = false;
};

} // namespace engine::core

#include "Engine/Core/EventBus.tpp"

#endif /* !EVENTBUS_HPP_ */
