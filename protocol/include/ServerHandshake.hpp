/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ServerHandshake.hpp
*/

#ifndef SERVERHANDSHAKE_HPP_
#define SERVERHANDSHAKE_HPP_

#include <cstdint>
#include <string>
#include <string_view>

#include "CookieFactory.hpp"
#include "Messages.hpp"
#include "RateLimiter.hpp"
#include "Timing.hpp"

namespace rtype::protocol {

/**
 * @brief Server side of the handshake for addresses that have no connection.
 *
 * It answers with a challenge until the client proves it owns its address
 * with the cookie, then asks the caller to create the connection. It keeps
 * no state per address before that, and never answers with more bytes than
 * it received.
 */
class ServerHandshake {
public:
    enum class Kind : std::uint8_t {
        DROP,
        REPLY,
        ACCEPT
    };

    struct Decision {
        Kind kind = Kind::DROP;
        std::string reply;
        Hello hello;
    };

    explicit ServerHandshake(const SipKey &secret);

    /**
     * @brief Handles a datagram from an address with no connection.
     *
     * @param address Text identifying the source, such as "ip:port".
     * @param nowSeconds Wall time in seconds, used to age the cookies.
     * @param slotAvailable Whether the game can take one more player.
     * @return REPLY with the datagram to send back, ACCEPT with the hello
     * of a client whose cookie is valid, or DROP.
     */
    [[nodiscard]] Decision handle(std::string_view address,
                                  std::string_view datagram, TimePoint now,
                                  std::uint64_t nowSeconds, bool slotAvailable);

    /**
     * @brief Returns a random non-zero session number.
     */
    [[nodiscard]] static std::uint32_t newSession();

private:
    [[nodiscard]] Decision onHello(std::string_view address, const Hello &hello,
                                   TimePoint now, std::uint64_t nowSeconds,
                                   bool slotAvailable);
    [[nodiscard]] Decision onWrongVersion(std::string_view address,
                                          std::string_view datagram,
                                          TimePoint now);
    [[nodiscard]] static Decision reject(RejectReason reason);

    CookieFactory _cookies;
    RateLimiter _helloLimiter;
    RateLimiter _versionLimiter;
};

} /* namespace rtype::protocol */

#endif /* !SERVERHANDSHAKE_HPP_ */
