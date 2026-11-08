/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** InputQueue.cpp
*/

#include "InputQueue.hpp"

#include "Protocol.hpp"
#include "SerialNumber.hpp"

namespace rtype::server {

void InputQueue::push(const protocol::Input &input)
{
    for (std::size_t i = 0; i < input.keys.size(); ++i) {
        const auto id = static_cast<std::uint16_t>(input.firstId + i);

        if (!protocol::isNewer(id, _lastQueued) ||
            _pending.size() >= protocol::MAX_INPUTS) {
            continue;
        }
        _pending.emplace_back(id, input.keys[i]);
        _lastQueued = id;
    }
}

std::uint8_t InputQueue::next()
{
    if (!_pending.empty()) {
        _applied = _pending.front().first;
        _held = _pending.front().second;
        _pending.pop_front();
    }
    return _held;
}

std::uint16_t InputQueue::applied() const noexcept
{
    return _applied;
}

std::size_t InputQueue::pending() const noexcept
{
    return _pending.size();
}

} /* namespace rtype::server */
