/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReliableReceiver.cpp
*/

#include "ReliableReceiver.hpp"

#include <utility>

#include "Protocol.hpp"
#include "SerialNumber.hpp"

namespace rtype::protocol {

bool ReliableReceiver::inWindow(std::uint16_t relId) const noexcept
{
    const bool delivered = isNewer(_next, relId);

    return delivered || distance(relId, _next) < MAX_OUTSTANDING_RELIABLE;
}

ReliableVerdict ReliableReceiver::accept(ReliableRecord record,
                                         std::vector<ReliableRecord> &delivered)
{
    if (!inWindow(record.relId)) {
        return ReliableVerdict::OUT_OF_WINDOW;
    }
    if (isNewer(_next, record.relId)) {
        return ReliableVerdict::DUPLICATE;
    }
    if (record.relId != _next) {
        const auto result = _buffer.emplace(record.relId, std::move(record));

        return result.second ? ReliableVerdict::BUFFERED
                             : ReliableVerdict::DUPLICATE;
    }
    delivered.push_back(std::move(record));
    ++_next;
    drain(delivered);
    return ReliableVerdict::DELIVERED;
}

void ReliableReceiver::drain(std::vector<ReliableRecord> &delivered)
{
    for (auto it = _buffer.find(_next); it != _buffer.end();
         it = _buffer.find(_next)) {
        delivered.push_back(std::move(it->second));
        _buffer.erase(it);
        ++_next;
    }
}

std::uint16_t ReliableReceiver::nextExpected() const noexcept
{
    return _next;
}

std::size_t ReliableReceiver::buffered() const noexcept
{
    return _buffer.size();
}

} /* namespace rtype::protocol */
