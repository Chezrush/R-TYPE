/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LocalAddress.hpp
*/

#ifndef LOCALADDRESS_HPP_
#define LOCALADDRESS_HPP_

#include <string>
#include <vector>

namespace rtype::server {

/**
 * @brief Lists the IPv4 addresses other computers can reach this one on.
 *
 * The first entry is the address of the interface used to leave the machine,
 * found without sending any packet. On Linux and macOS every active
 * non-loopback IPv4 interface follows, so a machine with several networks
 * (Wi-Fi, Ethernet, virtual) lists all of them. Loopback addresses are never
 * listed and the list is empty when the machine has no network.
 */
[[nodiscard]] std::vector<std::string> localAddresses();

/**
 * @brief Returns true if text is a dotted IPv4 address that is not loopback.
 */
[[nodiscard]] bool isReachableAddress(const std::string &text);

} /* namespace rtype::server */

#endif /* !LOCALADDRESS_HPP_ */
