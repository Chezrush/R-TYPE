/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** MessageQueue
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
 *
 */
template <typename T>
class MessageQueue {
public:
    explicit MessageQueue(std::size_t max) : _max(max) {}

    /**
     * @brief Adds a message at the back of the queue.
     *
     * @return true if the message was stored, false if the queue was full.
     */
    bool handleEnqueue(T message);

    /**
     * @brief Removes the oldest message from the queue.
     *
     * @return The message, or std::nullopt if the queue was empty.
     */
    [[nodiscard]] std::optional<T> handleDequeue();
    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool full() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] std::size_t getMax() const noexcept;
    
private:
    std::size_t _capacity;
    mutable std::mutex _mutex;
    std::deque<T> _messages;

};


} /* namespace rtype::server */

#endif /* MESSAGEQUEUE_HPP_ */
