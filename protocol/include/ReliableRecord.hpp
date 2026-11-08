/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReliableRecord.hpp
*/

#ifndef RELIABLERECORD_HPP_
#define RELIABLERECORD_HPP_

#include <cstdint>
#include <string>

namespace rtype::protocol {

/**
 * @brief A record of the reliable channel: its rel_id, its type and its
 * body without the rel_id.
 */
struct ReliableRecord {
    std::uint16_t relId = 0;
    std::uint8_t type = 0;
    std::string payload;

    friend bool operator==(const ReliableRecord &,
                           const ReliableRecord &) = default;
};

} /* namespace rtype::protocol */

#endif /* !RELIABLERECORD_HPP_ */
