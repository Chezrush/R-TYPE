/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CookieFactory.hpp
*/

#ifndef COOKIEFACTORY_HPP_
#define COOKIEFACTORY_HPP_

#include <cstdint>
#include <string_view>

#include "SipHash.hpp"

namespace rtype::protocol {

/**
 * @brief Stateless cookies for the handshake.
 *
 * A cookie is a keyed hash of the address of the client and of a coarse
 * time slot, so the server keeps nothing for an address that has not
 * answered yet. It is valid during the slot it was made in and the next one.
 */
class CookieFactory {
public:
    static constexpr std::uint64_t SLOT_SECONDS = 5;

    explicit CookieFactory(const SipKey &secret) noexcept;

    /**
     * @brief Builds a key from the system random generator.
     */
    static SipKey randomSecret();

    [[nodiscard]] std::uint64_t make(std::string_view address,
                                     std::uint64_t nowSeconds) const noexcept;

    [[nodiscard]] bool verify(std::string_view address, std::uint64_t cookie,
                              std::uint64_t nowSeconds) const noexcept;

private:
    [[nodiscard]] std::uint64_t hash(std::string_view address,
                                     std::uint64_t slot) const noexcept;

    SipKey _secret;
};

} /* namespace rtype::protocol */

#endif /* !COOKIEFACTORY_HPP_ */
