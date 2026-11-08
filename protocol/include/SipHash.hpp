/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SipHash.hpp
*/

#ifndef SIPHASH_HPP_
#define SIPHASH_HPP_

#include <array>
#include <cstdint>
#include <string_view>

namespace rtype::protocol {

using SipKey = std::array<std::uint8_t, 16>;

/**
 * @brief SipHash-2-4, a keyed hash made for short inputs.
 *
 * It is used to derive the handshake cookies, so that nobody can forge one
 * without the secret key of the server.
 */
std::uint64_t sipHash24(const SipKey &key, std::string_view data) noexcept;

} /* namespace rtype::protocol */

#endif /* !SIPHASH_HPP_ */
