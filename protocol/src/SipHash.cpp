/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SipHash.cpp
*/

#include "SipHash.hpp"

namespace rtype::protocol {

namespace {

struct SipState {
    std::uint64_t v0;
    std::uint64_t v1;
    std::uint64_t v2;
    std::uint64_t v3;
};

std::uint64_t rotl(std::uint64_t value, unsigned bits) noexcept
{
    return (value << bits) | (value >> (64U - bits));
}

template <typename Bytes>
std::uint64_t loadLittleEndian(const Bytes &bytes, std::size_t offset,
                               std::size_t count) noexcept
{
    std::uint64_t value = 0;

    for (std::size_t i = 0; i < count; ++i) {
        const auto byte = static_cast<std::uint8_t>(bytes[offset + i]);

        value |= static_cast<std::uint64_t>(byte) << (8U * i);
    }
    return value;
}

SipState initialState(const SipKey &key) noexcept
{
    const std::uint64_t k0 = loadLittleEndian(key, 0, 8);
    const std::uint64_t k1 = loadLittleEndian(key, 8, 8);

    return {k0 ^ 0x736f6d6570736575ULL, k1 ^ 0x646f72616e646f6dULL,
            k0 ^ 0x6c7967656e657261ULL, k1 ^ 0x7465646279746573ULL};
}

void sipRound(SipState &s) noexcept
{
    s.v0 += s.v1;
    s.v1 = rotl(s.v1, 13);
    s.v1 ^= s.v0;
    s.v0 = rotl(s.v0, 32);
    s.v2 += s.v3;
    s.v3 = rotl(s.v3, 16);
    s.v3 ^= s.v2;
    s.v0 += s.v3;
    s.v3 = rotl(s.v3, 21);
    s.v3 ^= s.v0;
    s.v2 += s.v1;
    s.v1 = rotl(s.v1, 17);
    s.v1 ^= s.v2;
    s.v2 = rotl(s.v2, 32);
}

void absorb(SipState &s, std::uint64_t word) noexcept
{
    s.v3 ^= word;
    sipRound(s);
    sipRound(s);
    s.v0 ^= word;
}

} // namespace

std::uint64_t sipHash24(const SipKey &key, std::string_view data) noexcept
{
    SipState s = initialState(key);
    const std::size_t whole = data.size() - data.size() % 8;

    for (std::size_t i = 0; i < whole; i += 8) {
        absorb(s, loadLittleEndian(data, i, 8));
    }
    const std::uint64_t last =
        loadLittleEndian(data, whole, data.size() - whole) |
        (static_cast<std::uint64_t>(data.size()) << 56U);

    absorb(s, last);
    s.v2 ^= 0xffU;
    for (int i = 0; i < 4; ++i) {
        sipRound(s);
    }
    return s.v0 ^ s.v1 ^ s.v2 ^ s.v3;
}

} /* namespace rtype::protocol */
