/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RateLimiter.cpp
*/

#include "RateLimiter.hpp"

#include <algorithm>

namespace rtype::protocol {

RateLimiter::RateLimiter(double perSecond, double burst, std::size_t maxKeys)
    : _perSecond(perSecond),
      _burst(burst),
      _maxKeys(maxKeys)
{}

void RateLimiter::evictIdle(TimePoint now)
{
    std::erase_if(_buckets, [this, now](const auto &item) {
        const std::chrono::duration<double> idle = now - item.second.updated;

        return item.second.tokens + idle.count() * _perSecond >= _burst;
    });
}

bool RateLimiter::allow(std::string_view key, TimePoint now)
{
    auto found = _buckets.find(std::string(key));

    if (found == _buckets.end()) {
        if (_buckets.size() >= _maxKeys) {
            evictIdle(now);
        }
        if (_buckets.size() >= _maxKeys) {
            return false;
        }
        found = _buckets.emplace(std::string(key), Bucket{_burst, now}).first;
    }
    Bucket &bucket = found->second;
    const std::chrono::duration<double> elapsed = now - bucket.updated;

    bucket.tokens =
        std::min(_burst, bucket.tokens + elapsed.count() * _perSecond);
    bucket.updated = now;
    if (bucket.tokens < 1.0) {
        return false;
    }
    bucket.tokens -= 1.0;
    return true;
}

std::size_t RateLimiter::size() const noexcept
{
    return _buckets.size();
}

} /* namespace rtype::protocol */
