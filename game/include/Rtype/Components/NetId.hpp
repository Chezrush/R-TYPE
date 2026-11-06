/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NetId.hpp
*/

#ifndef NETID_HPP_
#define NETID_HPP_

#include <cstdint>

namespace rtype::game {

/**
 * @brief Id of an entity on the network, the net_id of the protocol.
 *
 * Unlike the engine Entity handle, it is only 16 bits and is the same for
 * every client.
 */
struct NetId {
    std::uint16_t value = 0;

    friend bool operator==(const NetId &, const NetId &) = default;
};

} /* namespace rtype::game */

#endif /* !NETID_HPP_ */
