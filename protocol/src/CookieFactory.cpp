/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CookieFactory.cpp
*/

#include "CookieFactory.hpp"

#include <random>
#include <string>

#include "ByteWriter.hpp"

namespace rtype::protocol {

CookieFactory::CookieFactory(const SipKey &secret) noexcept
    : _secret(secret)
{}

SipKey CookieFactory::randomSecret()
{
    std::random_device device;
    SipKey key{};

    for (auto &byte : key) {
        byte = static_cast<std::uint8_t>(device());
    }
    return key;
}

std::uint64_t CookieFactory::hash(std::string_view address,
                                  std::uint64_t slot) const noexcept
{
    std::string input;

    ByteWriter(input).writeU64(slot).writeBytes(address);
    return sipHash24(_secret, input);
}

std::uint64_t CookieFactory::make(std::string_view address,
                                  std::uint64_t nowSeconds) const noexcept
{
    const std::uint64_t cookie = hash(address, nowSeconds / SLOT_SECONDS);

    return cookie == 0 ? 1 : cookie;
}

bool CookieFactory::verify(std::string_view address, std::uint64_t cookie,
                           std::uint64_t nowSeconds) const noexcept
{
    const std::uint64_t slot = nowSeconds / SLOT_SECONDS;

    return cookie != 0 &&
           (cookie == make(address, nowSeconds) ||
            (slot > 0 && cookie == make(address, nowSeconds - SLOT_SECONDS)));
}

} /* namespace rtype::protocol */
