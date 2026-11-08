/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Messages.cpp
*/

#include "Messages.hpp"

namespace rtype::protocol {

bool isReliable(std::uint8_t type) noexcept
{
    return (type & RELIABLE_BIT) != 0;
}

bool isReliable(MessageType type) noexcept
{
    return isReliable(static_cast<std::uint8_t>(type));
}

MessageType typeOf(const Message &message)
{
    return std::visit([](const auto &value) { return value.TYPE; }, message);
}

} /* namespace rtype::protocol */
