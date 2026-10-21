/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** MessageQueue
*/

#include "MessageQueue.hpp"

bool MessageQueue::handleEnqueue(T message)
{
    std::lock_guard lock(_mutex);

    if (_messages.size() >= _max) {
        return false;
    }
    _messages.push_back(std::move(message));
    return true;
}

std::optional<T> MessageQueue::handleDequeue()
{
    std::lock_guard lock(_mutex);

    if (_messages.empty()) {
        return std::nullopt;
    }

    auto message = std::move(_messages.front());
    _messages.pop_front();
    return message;
}

bool MessageQueue::empty() const
{
    std::lock_guard lock(_mutex);
    return _messages.empty();
}

bool MessageQueue::full() const
{
    std::lock_guard lock(_mutex);
    return _messages.size() >= _max;
}

std::size_t MessageQueue::size() const
{
    std::lock_guard lock(_mutex);
    return _messages.size();
}

std::size_t MessageQueue::getMax() const noexcept
{
    return _max;
}

MessageQueue::~MessageQueue()
{

}
