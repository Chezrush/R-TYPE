/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RateLimiter.hpp
*/

#ifndef RATELIMITER_HPP_
#define RATELIMITER_HPP_

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Timing.hpp"

namespace rtype::protocol {

/**
 * @brief Token bucket per key (an address), with a bounded table.
 *
 * Used for the hello flood and for the datagram rate of a connection.
 */
class RateLimiter {
public:
    /**
     * @param perSecond Tokens given back each second.
     * @param burst Tokens a key can store.
     * @param maxKeys Size of the table, a full table refuses new keys.
     */
    RateLimiter(double perSecond, double burst, std::size_t maxKeys);

    /**
     * @brief Takes a token for the key.
     *
     * @return false if the key has none left, or if the table is full of
     * recently active keys.
     */
    [[nodiscard]] bool allow(std::string_view key, TimePoint now);

    [[nodiscard]] std::size_t size() const noexcept;

private:
    struct Bucket {
        double tokens = 0;
        TimePoint updated;
    };

    void evictIdle(TimePoint now);

    double _perSecond;
    double _burst;
    std::size_t _maxKeys;
    std::unordered_map<std::string, Bucket> _buckets;
};

} /* namespace rtype::protocol */

#endif /* !RATELIMITER_HPP_ */
