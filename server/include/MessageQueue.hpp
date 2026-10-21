/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MessageQueue.hpp
*/

#ifndef MESSAGEQUEUE_HPP_
#define MESSAGEQUEUE_HPP_

#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

namespace rtype::server {

/**
 * @brief Thread-safe, capacity-bounded FIFO queue.
 *
 * All operations are non-blocking. A failed enqueue means that the queue
 * is full; a failed dequeue means that it is empty.
 */
template <typename T>
class MessageQueue {
public:
    explicit MessageQueue(std::size_t capacity) : _capacity(capacity){}

    /**
     * @brief Adds a message at the back of the queue.
     *
     * @return true if the message was stored, false if the queue was full.
     */
    bool handleEnqueue(T message)
    {
        const std::lock_guard<std::mutex> lock(_mutex);

        if (_messages.size() >= _capacity) {
            return false;
        }
        _messages.push_back(std::move(message));
        return true;
    }

    /**
     * @brief Removes the oldest message from the queue.
     *
     * @return The message, or std::nullopt if the queue was empty.
     */
    [[nodiscard]] std::optional<T> handleDequeue()
    {
        const std::lock_guard<std::mutex> lock(_mutex);

        if (_messages.empty()) {
            return std::nullopt;
        }
        auto message = std::move(_messages.front());
        _messages.pop_front();
        return message;
    }

    [[nodiscard]] bool empty() const
    {
        const std::lock_guard<std::mutex> lock(_mutex);

        return _messages.empty();
    }

    [[nodiscard]] bool full() const
    {
        const std::lock_guard<std::mutex> lock(_mutex);

        return _messages.size() >= _capacity;
    }

    [[nodiscard]] std::size_t size() const
    {
        const std::lock_guard<std::mutex> lock(_mutex);

        return _messages.size();
    }

    [[nodiscard]] std::size_t getMax() const noexcept
    {
        return _capacity;
    }

private:
    std::size_t _capacity;
    mutable std::mutex _mutex;
    std::deque<T> _messages;
};

} /* namespace rtype::server */

#endif /* !MESSAGEQUEUE_HPP_ */