/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** InputBuffer
*/

#include "InputBuffer.hpp"

#include <algorithm>

#include "SerialNumber.hpp"

namespace rtype::server {

void InputBuffer::push(const protocol::Input &input)
{
    if (!_started) {
        _lastApplied = static_cast<std::uint16_t>(input.firstId - 1);
        _started = true;
    }
    for (std::size_t i = 0; i < input.keys.size(); ++i) {
        store(static_cast<std::uint16_t>(input.firstId + i), input.keys[i]);
    }
}

void InputBuffer::store(std::uint16_t id, std::uint8_t keys)
{
    if (!protocol::isNewer(id, _lastApplied)) {
        return;
    }
    const std::size_t index = protocol::distance(id, _lastApplied) - 1U;

    if (index < _pending.size()) {
        _pending.at(index) = keys;
    }
}

std::optional<std::uint8_t> InputBuffer::pop()
{
    const auto oldest = std::ranges::find_if(
        _pending, [](const auto &sample) { return sample.has_value(); });

    if (oldest == _pending.end()) {
        return std::nullopt;
    }
    const std::uint8_t keys = oldest->value();
    const auto taken = oldest - _pending.begin() + 1;

    _lastApplied = static_cast<std::uint16_t>(_lastApplied + taken);
    std::shift_left(_pending.begin(), _pending.end(), taken);
    std::fill(_pending.end() - taken, _pending.end(), std::nullopt);
    return keys;
}

std::uint16_t InputBuffer::lastApplied() const noexcept
{
    return _lastApplied;
}

} /* namespace rtype::server */
